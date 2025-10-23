#include "user_controller.h"
#include <jwt-cpp/jwt.h>
#include <drogon/orm/DbClient.h>
#include <drogon/drogon.h>
#include <chrono>

using namespace api::v1;

Json::Value ApiResponse::toJson() const {
    Json::Value json;
    json["success"] = success;
    json["message"] = message;
    json["data"] = data;
    return json;
}

std::string JwtUtil::generateAccessToken(int userId, const std::string& email, 
                                        const std::string& login, const std::string& role) {
    try {
        auto token = jwt::create()
            .set_type("JWT")
            .set_issued_at(std::chrono::system_clock::now())
            .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds(JwtConfig::ACCESS_TOKEN_EXPIRY))
            .set_payload_claim("user_id", jwt::claim(std::to_string(userId)))
            .set_payload_claim("email", jwt::claim(email))
            .set_payload_claim("login", jwt::claim(login))
            .set_payload_claim("role", jwt::claim(role))
            .sign(jwt::algorithm::hs256{JwtConfig::SECRET_KEY});
        
        LOG_DEBUG << "Generated access token for user_id=" << userId;
        return token;
    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to generate access token: " << e.what();
        return "";
    }
}

std::string JwtUtil::generateRefreshToken(int userId) {
    try {
        auto token = jwt::create()
            .set_type("JWT")
            .set_issued_at(std::chrono::system_clock::now())
            .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds(JwtConfig::REFRESH_TOKEN_EXPIRY))
            .set_payload_claim("user_id", jwt::claim(std::to_string(userId)))
            .set_payload_claim("type", jwt::claim(std::string("refresh")))
            .sign(jwt::algorithm::hs256{JwtConfig::SECRET_KEY});
        
        LOG_DEBUG << "Generated refresh token for user_id=" << userId;
        return token;
    } catch (const std::exception& e) {
        LOG_ERROR << "Failed to generate refresh token: " << e.what();
        return "";
    }
}

bool JwtUtil::validateToken(const std::string& token, Json::Value& claims) {
    try {
        LOG_DEBUG << "Starting token validation. Token length: " << token.length();
        
        // Проверяем что токен имеет формат JWT (3 части разделены точками)
        int dot_count = std::count(token.begin(), token.end(), '.');
        if (dot_count != 2) {
            LOG_ERROR << "Invalid token format. Expected 2 dots, found " << dot_count << ". Token: " << token;
            return false;
        }

        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{JwtConfig::SECRET_KEY});
        
        verifier.verify(decoded);

        claims["user_id"] = std::stoi(decoded.get_payload_claim("user_id").as_string());
        claims["email"] = decoded.get_payload_claim("email").as_string();
        claims["login"] = decoded.get_payload_claim("login").as_string();
        claims["role"] = decoded.get_payload_claim("role").as_string();

        LOG_DEBUG << "Token validated successfully for user_id=" << claims["user_id"].asInt();
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR << "Token validation failed: " << e.what();
        return false;
    }
}

std::string JwtUtil::extractTokenFromHeader(const HttpRequestPtr& req) {
    auto authHeader = req->getHeader("Authorization");
    LOG_DEBUG << "Authorization header: " << (authHeader.empty() ? "empty" : authHeader.substr(0, 50) + "...");
    
    if (authHeader.empty()) {
        return "";
    }

    const std::string bearer = "Bearer ";
    if (authHeader.substr(0, bearer.length()) == bearer) {
        std::string token = authHeader.substr(bearer.length());
        LOG_DEBUG << "Extracted token from header. Length: " << token.length();
        return token;
    }
    LOG_WARN << "Authorization header does not start with 'Bearer '";
    return "";
}

User::User() {
    LOG_DEBUG << "User controller initialized";
}

void User::registerUser(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json) {
        auto resp = HttpResponse::newHttpJsonResponse(createErrorResponse("Invalid JSON"));
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }
    if (!json->isMember("email") || !json->isMember("login") || !json->isMember("password")) {
        auto resp = HttpResponse::newHttpJsonResponse(
            createErrorResponse("Missing required fields: email, login, password"));
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }
    std::string email = (*json)["email"].asString();
    std::string login = (*json)["login"].asString();
    std::string password = (*json)["password"].asString();
    std::string phone = json->isMember("phone") ? (*json)["phone"].asString() : "";

    auto dbClient = app().getDbClient();
    *dbClient << "SELECT * FROM register_user($1, $2, $3, $4, $5)"
        << email << login << password << phone << 3
        >> [callback](const orm::Result& r) {
            if (r.empty()) {
                auto resp = HttpResponse::newHttpJsonResponse(
                    createErrorResponse("Registration failed"));
                resp->setStatusCode(k500InternalServerError);
                callback(resp);
                return;
            }
            bool success = r[0]["success"].as<bool>();
            std::string message = r[0]["message"].as<std::string>();

            if (success) {
                int userId = r[0]["user_id"].as<int>();
                Json::Value data;
                data["user_id"] = userId;

                auto resp = HttpResponse::newHttpJsonResponse(
                    createSuccessResponse("User registered successfully", data));
                resp->setStatusCode(k201Created);
                callback(resp);
            } else {
                auto resp = HttpResponse::newHttpJsonResponse(
                    createErrorResponse(message));
                resp->setStatusCode(k400BadRequest);
                callback(resp);
            }
        }
        >> [callback](const orm::DrogonDbException& e) {
            LOG_ERROR << "Database error: " << e.base().what();
            auto resp = HttpResponse::newHttpJsonResponse(
                createErrorResponse("Database error: " + std::string(e.base().what())));
            resp->setStatusCode(k500InternalServerError);
            callback(resp);
        };
}

void User::login(const HttpRequestPtr& req,
                std::function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json) {
        auto resp = HttpResponse::newHttpJsonResponse(createErrorResponse("Invalid JSON"));
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }
    if (!json->isMember("login") || !json->isMember("password")) {
        auto resp = HttpResponse::newHttpJsonResponse(
            createErrorResponse("Missing required fields: login, password"));
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }
    std::string login = (*json)["login"].asString();
    std::string password = (*json)["password"].asString();
    std::string ipAddress = req->peerAddr().toIp();
    std::string userAgent = req->getHeader("User-Agent");

    auto dbClient = app().getDbClient();
    *dbClient << "SELECT * FROM authenticate_user($1, $2, $3, $4)"
        << login << password << ipAddress << userAgent
        >> [callback, ipAddress, userAgent](const orm::Result& r) {
            if (r.empty()) {
                auto resp = HttpResponse::newHttpJsonResponse(
                    createErrorResponse("Authentication failed"));
                resp->setStatusCode(k401Unauthorized);
                callback(resp);
                return;
            }
            bool success = r[0]["success"].as<bool>();
            std::string message = r[0]["message"].as<std::string>();

            if (success) {
                int userId = r[0]["user_id"].as<int>();
                std::string email = r[0]["email"].as<std::string>();
                std::string loginData = r[0]["login"].as<std::string>();
                std::string roleName = r[0]["role_name"].as<std::string>();

                // Генерируем токены
                std::string accessToken = JwtUtil::generateAccessToken(userId, email, loginData, roleName);
                std::string refreshToken = JwtUtil::generateRefreshToken(userId);

                if (accessToken.empty() || refreshToken.empty()) {
                    auto resp = HttpResponse::newHttpJsonResponse(
                        createErrorResponse("Failed to generate tokens"));
                    resp->setStatusCode(k500InternalServerError);
                    callback(resp);
                    return;
                }

                Json::Value data;
                data["user_id"] = userId;
                data["email"] = email;
                data["login"] = loginData;
                data["role"] = roleName;
                data["access_token"] = accessToken;
                data["refresh_token"] = refreshToken;

                auto resp = HttpResponse::newHttpJsonResponse(
                    createSuccessResponse("Login successful", data));
                resp->setStatusCode(k200OK);
                callback(resp);
            } else {
                auto resp = HttpResponse::newHttpJsonResponse(
                    createErrorResponse(message));
                resp->setStatusCode(k401Unauthorized);
                callback(resp);
            }
        }
        >> [callback](const orm::DrogonDbException& e) {
            LOG_ERROR << "Database error: " << e.base().what();
            auto resp = HttpResponse::newHttpJsonResponse(
                createErrorResponse("Database error"));
            resp->setStatusCode(k500InternalServerError);
            callback(resp);
        };
}

void User::refreshToken(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json) {
        auto resp = HttpResponse::newHttpJsonResponse(createErrorResponse("Invalid JSON"));
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }
    if (!json->isMember("refresh_token")) {
        auto resp = HttpResponse::newHttpJsonResponse(
            createErrorResponse("Missing refresh_token"));
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }
    std::string refreshToken = (*json)["refresh_token"].asString();
    std::string ipAddress = req->peerAddr().toIp();
    std::string userAgent = req->getHeader("User-Agent");

    auto dbClient = app().getDbClient();
    *dbClient << "SELECT * FROM refresh_session($1, $2, $3)"
        << refreshToken << ipAddress << userAgent
        >> [callback](const orm::Result& r) {
            if (r.empty()) {
                auto resp = HttpResponse::newHttpJsonResponse(
                    createErrorResponse("Token refresh failed"));
                resp->setStatusCode(k401Unauthorized);
                callback(resp);
                return;
            }
            bool success = r[0]["success"].as<bool>();
            std::string message = r[0]["message"].as<std::string>();

            if (success) {
                std::string newAccessToken = r[0]["new_access_token"].as<std::string>();
                std::string newRefreshToken = r[0]["new_refresh_token"].as<std::string>();

                Json::Value data;
                data["access_token"] = newAccessToken;
                data["refresh_token"] = newRefreshToken;

                auto resp = HttpResponse::newHttpJsonResponse(
                    createSuccessResponse("Tokens refreshed", data));
                resp->setStatusCode(k200OK);
                callback(resp);
            } else {
                auto resp = HttpResponse::newHttpJsonResponse(
                    createErrorResponse(message));
                resp->setStatusCode(k401Unauthorized);
                callback(resp);
            }
        }
        >> [callback](const orm::DrogonDbException& e) {
            LOG_ERROR << "Database error: " << e.base().what();
            auto resp = HttpResponse::newHttpJsonResponse(
                createErrorResponse("Database error"));
            resp->setStatusCode(k500InternalServerError);
            callback(resp);
        };
}

void User::getUserInfo(const HttpRequestPtr& req,
                      std::function<void(const HttpResponsePtr&)>&& callback,
                      int id) {
    auto authResult = authenticateRequest(req);
    if (!authResult.success) {
        auto resp = HttpResponse::newHttpJsonResponse(
            createErrorResponse(authResult.message));
        resp->setStatusCode(k401Unauthorized);
        callback(resp);
        return;
    }
    if (authResult.userId != id && authResult.role != "admin") {
        auto resp = HttpResponse::newHttpJsonResponse(
            createErrorResponse("Access denied"));
        resp->setStatusCode(k403Forbidden);
        callback(resp);
        return;
    }
    auto dbClient = app().getDbClient();
    *dbClient << "SELECT * FROM get_user_info($1)"
        << id
        >> [callback](const orm::Result& r) {
            if (r.empty()) {
                auto resp = HttpResponse::newHttpJsonResponse(
                    createErrorResponse("User not found"));
                resp->setStatusCode(k404NotFound);
                callback(resp);
                return;
            }
            Json::Value data;
            data["user_id"] = r[0]["user_id"].as<int>();
            data["email"] = r[0]["email"].as<std::string>();
            data["login"] = r[0]["login"].as<std::string>();
            data["phone"] = r[0]["phone"].as<std::string>();
            data["is_confirmed"] = r[0]["is_confirmed"].as<bool>();
            data["is_profile_active"] = r[0]["is_profile_active"].as<bool>();
            data["role_name"] = r[0]["role_name"].as<std::string>();

            auto resp = HttpResponse::newHttpJsonResponse(
                createSuccessResponse("User info retrieved", data));
            resp->setStatusCode(k200OK);
            callback(resp);
        }
        >> [callback](const orm::DrogonDbException& e) {
            LOG_ERROR << "Database error: " << e.base().what();
            auto resp = HttpResponse::newHttpJsonResponse(
                createErrorResponse("Database error"));
            resp->setStatusCode(k500InternalServerError);
            callback(resp);
        };
}

void User::logout(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& callback) {
    auto authResult = authenticateRequest(req);
    if (!authResult.success) {
        auto resp = HttpResponse::newHttpJsonResponse(
            createErrorResponse(authResult.message));
        resp->setStatusCode(k401Unauthorized);
        callback(resp);
        return;
    }
    std::string token = JwtUtil::extractTokenFromHeader(req);
    auto dbClient = app().getDbClient();
    *dbClient << "SELECT * FROM logout_session($1)"
        << token
        >> [callback](const orm::Result& r) {
            if (r.empty()) {
                auto resp = HttpResponse::newHttpJsonResponse(
                    createErrorResponse("Logout failed"));
                resp->setStatusCode(k500InternalServerError);
                callback(resp);
                return;
            }
            auto resp = HttpResponse::newHttpJsonResponse(
                createSuccessResponse("Logout successful"));
            resp->setStatusCode(k200OK);
            callback(resp);
        }
        >> [callback](const orm::DrogonDbException& e) {
            LOG_ERROR << "Database error: " << e.base().what();
            auto resp = HttpResponse::newHttpJsonResponse(
                createErrorResponse("Database error"));
            resp->setStatusCode(k500InternalServerError);
            callback(resp);
        };
}

void User::changePassword(const HttpRequestPtr& req,
                         std::function<void(const HttpResponsePtr&)>&& callback,
                         int id) {
    auto authResult = authenticateRequest(req);
    if (!authResult.success) {
        auto resp = HttpResponse::newHttpJsonResponse(
            createErrorResponse(authResult.message));
        resp->setStatusCode(k401Unauthorized);
        callback(resp);
        return;
    }
    if (authResult.userId != id && authResult.role != "admin") {
        auto resp = HttpResponse::newHttpJsonResponse(
            createErrorResponse("Access denied"));
        resp->setStatusCode(k403Forbidden);
        callback(resp);
        return;
    }
    auto json = req->getJsonObject();
    if (!json) {
        auto resp = HttpResponse::newHttpJsonResponse(createErrorResponse("Invalid JSON"));
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }
    if (!json->isMember("old_password") || !json->isMember("new_password")) {
        auto resp = HttpResponse::newHttpJsonResponse(
            createErrorResponse("Missing required fields"));
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }
    std::string oldPassword = (*json)["old_password"].asString();
    std::string newPassword = (*json)["new_password"].asString();

    auto dbClient = app().getDbClient();
    *dbClient << "SELECT * FROM change_password($1, $2, $3)"
        << id << oldPassword << newPassword
        >> [callback](const orm::Result& r) {
            if (r.empty()) {
                auto resp = HttpResponse::newHttpJsonResponse(
                    createErrorResponse("Change password failed"));
                resp->setStatusCode(k500InternalServerError);
                callback(resp);
                return;
            }
            bool success = r[0]["success"].as<bool>();
            std::string message = r[0]["message"].as<std::string>();
            if (success) {
                auto resp = HttpResponse::newHttpJsonResponse(
                    createSuccessResponse(message));
                resp->setStatusCode(k200OK);
                callback(resp);
            } else {
                auto resp = HttpResponse::newHttpJsonResponse(
                    createErrorResponse(message));
                resp->setStatusCode(k400BadRequest);
                callback(resp);
            }
        }
        >> [callback](const orm::DrogonDbException& e) {
            LOG_ERROR << "Database error: " << e.base().what();
            auto resp = HttpResponse::newHttpJsonResponse(
                createErrorResponse("Database error"));
            resp->setStatusCode(k500InternalServerError);
            callback(resp);
        };
}

void User::getActiveSessions(const HttpRequestPtr& req,
                            std::function<void(const HttpResponsePtr&)>&& callback,
                            int id) {
    auto authResult = authenticateRequest(req);
    if (!authResult.success) {
        auto resp = HttpResponse::newHttpJsonResponse(
            createErrorResponse(authResult.message));
        resp->setStatusCode(k401Unauthorized);
        callback(resp);
        return;
    }
    if (authResult.userId != id && authResult.role != "admin") {
        auto resp = HttpResponse::newHttpJsonResponse(
            createErrorResponse("Access denied"));
        resp->setStatusCode(k403Forbidden);
        callback(resp);
        return;
    }
    auto dbClient = app().getDbClient();
    *dbClient << "SELECT * FROM get_user_sessions($1)"
        << id
        >> [callback](const orm::Result& r) {
            Json::Value sessionsArray(Json::arrayValue);
            for (auto row : r) {
                Json::Value session;
                session["session_id"] = row["session_id"].as<int>();
                session["ip_address"] = row["ip_address"].as<std::string>();
                session["user_agent"] = row["user_agent"].as<std::string>();
                session["created_at"] = row["created_at"].as<std::string>();
                session["last_activity"] = row["last_activity"].as<std::string>();
                sessionsArray.append(session);
            }
            auto resp = HttpResponse::newHttpJsonResponse(
                createSuccessResponse("Active sessions retrieved", sessionsArray));
            resp->setStatusCode(k200OK);
            callback(resp);
        }
        >> [callback](const orm::DrogonDbException& e) {
            LOG_ERROR << "Database error: " << e.base().what();
            auto resp = HttpResponse::newHttpJsonResponse(
                createErrorResponse("Database error"));
            resp->setStatusCode(k500InternalServerError);
            callback(resp);
        };
}

User::AuthResult User::authenticateRequest(const HttpRequestPtr& req) {
    std::string token = JwtUtil::extractTokenFromHeader(req);
    if (token.empty()) {
        return {false, "Missing Authorization header", -1, ""};
    }
    Json::Value claims;
    if (!JwtUtil::validateToken(token, claims)) {
        return {false, "Invalid or expired token", -1, ""};
    }
    return {
        true,
        "Authentication successful",
        claims["user_id"].asInt(),
        claims["role"].asString()
    };
}

bool User::hasRole(const std::string& userRole, const std::string& requiredRole) {
    return userRole == requiredRole || userRole == "admin";
}

Json::Value User::createSuccessResponse(const std::string& message, 
                                        const Json::Value& data) {
    Json::Value response;
    response["success"] = true;
    response["message"] = message;
    response["data"] = data;
    return response;
}

Json::Value User::createErrorResponse(const std::string& message) {
    Json::Value response;
    response["success"] = false;
    response["message"] = message;
    response["data"] = Json::Value();
    return response;
}

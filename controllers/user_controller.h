// user_controller.h

#pragma once

#include <drogon/HttpController.h>
#include <drogon/orm/DbClient.h>
#include <json/json.h>
#include <string>
#include <memory>

using namespace drogon;

namespace api::v1 {

struct ApiResponse {
    bool success;
    std::string message;
    Json::Value data;

    Json::Value toJson() const;
};

struct JwtConfig {
    static constexpr const char* SECRET_KEY = "your_very_long_secret_key_minimum_32_characters_long";
    static constexpr const char* ALGORITHM = "HS256";
    static constexpr int ACCESS_TOKEN_EXPIRY = 1800;
    static constexpr int REFRESH_TOKEN_EXPIRY = 2592000;
};

class JwtUtil {
public:
    static std::string generateAccessToken(int userId, const std::string& email, 
                                          const std::string& login, const std::string& role);
    
    static std::string generateRefreshToken(int userId);
    
    static bool validateToken(const std::string& token, Json::Value& claims);
    
    static std::string extractTokenFromHeader(const HttpRequestPtr& req);

private:
    JwtUtil() = default;
};

class User : public drogon::HttpController<User> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(User::registerUser, "/api/v1/auth/register", Post);
    ADD_METHOD_TO(User::login, "/api/v1/auth/login", Post);
    ADD_METHOD_TO(User::refreshToken, "/api/v1/auth/refresh", Post);
    ADD_METHOD_TO(User::getUserInfo, "/api/v1/users/{id}", Get);
    ADD_METHOD_TO(User::logout, "/api/v1/auth/logout", Post);
    ADD_METHOD_TO(User::changePassword, "/api/v1/users/{id}/password", Post);
    ADD_METHOD_TO(User::getActiveSessions, "/api/v1/users/{id}/sessions", Get);
    METHOD_LIST_END

    User();

    void registerUser(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& callback);

    void login(const HttpRequestPtr& req,
              std::function<void(const HttpResponsePtr&)>&& callback);

    void refreshToken(const HttpRequestPtr& req,
                     std::function<void(const HttpResponsePtr&)>&& callback);

    void getUserInfo(const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)>&& callback,
                    int id);

    void logout(const HttpRequestPtr& req,
               std::function<void(const HttpResponsePtr&)>&& callback);

    void changePassword(const HttpRequestPtr& req,
                       std::function<void(const HttpResponsePtr&)>&& callback,
                       int id);

    void getActiveSessions(const HttpRequestPtr& req,
                          std::function<void(const HttpResponsePtr&)>&& callback,
                          int id);

private:
    struct AuthResult {
        bool success;
        std::string message;
        int userId;
        std::string role;
    };

    AuthResult authenticateRequest(const HttpRequestPtr& req);
    bool hasRole(const std::string& userRole, const std::string& requiredRole);
    static Json::Value createSuccessResponse(const std::string& message, 
                                            const Json::Value& data = Json::Value());
    static Json::Value createErrorResponse(const std::string& message);
};

} // namespace api::v1

#include <drogon/drogon.h>
int main() {
    drogon::app().loadConfigFile("../config.json");
    drogon::app().registerHandler("/register", [](const drogon::HttpRequestPtr& req,
                                                  std::function<void(const drogon::HttpResponsePtr&)>&& callback){
        auto resp = drogon::HttpResponse::newFileResponse("../views/register.html");
        resp->setContentTypeCode(drogon::ContentType::CT_TEXT_HTML);
        callback(resp);
    });

    drogon::app().registerHandler("/login", [](const drogon::HttpRequestPtr& req,
                                               std::function<void(const drogon::HttpResponsePtr&)>&& callback){
        auto resp = drogon::HttpResponse::newFileResponse("../views/login.html");
        resp->setContentTypeCode(drogon::ContentType::CT_TEXT_HTML);
        callback(resp);
    });
    drogon::app().run();
    return 0;
}

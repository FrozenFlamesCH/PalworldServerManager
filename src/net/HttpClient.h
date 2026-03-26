#pragma once

#include <string>
#include <map>
#include <functional>
#include <nlohmann/json.hpp>

namespace PSM {

struct HttpResponse {
    int statusCode = 0;
    std::string body;
    std::map<std::string, std::string> headers;
    std::string error;
    bool success = false;

    nlohmann::json json() const {
        try {
            if (!body.empty()) return nlohmann::json::parse(body);
        } catch (...) {}
        return nlohmann::json();
    }
};

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    HttpResponse get(const std::string& url,
                     const std::map<std::string, std::string>& headers = {});
    HttpResponse post(const std::string& url, const std::string& body = "",
                      const std::map<std::string, std::string>& headers = {});
    HttpResponse put(const std::string& url, const std::string& body = "",
                     const std::map<std::string, std::string>& headers = {});
    HttpResponse del(const std::string& url,
                     const std::map<std::string, std::string>& headers = {});

    void setTimeout(int seconds);
    void setBasicAuth(const std::string& username, const std::string& password);

private:
    HttpResponse perform(const std::string& method, const std::string& url,
                        const std::string& body, const std::map<std::string, std::string>& headers);

    static size_t writeCallback(char* ptr, size_t size, size_t nmemb, std::string* data);
    static size_t headerCallback(char* buffer, size_t size, size_t nitems, std::map<std::string, std::string>* headers);

    int m_timeout = 10;
    std::string m_basicAuth;
};

} // namespace PSM

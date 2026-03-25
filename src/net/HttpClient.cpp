#include "net/HttpClient.h"
#include <curl/curl.h>
#include <sstream>
#include <algorithm>

namespace PSM {

HttpClient::HttpClient() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

HttpClient::~HttpClient() {
    curl_global_cleanup();
}

size_t HttpClient::writeCallback(char* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

size_t HttpClient::headerCallback(char* buffer, size_t size, size_t nitems,
                                   std::map<std::string, std::string>* headers) {
    size_t totalSize = size * nitems;
    std::string line(buffer, totalSize);

    auto colonPos = line.find(':');
    if (colonPos != std::string::npos) {
        std::string key = line.substr(0, colonPos);
        std::string value = line.substr(colonPos + 1);
        // Trim
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);
        (*headers)[key] = value;
    }

    return totalSize;
}

HttpResponse HttpClient::perform(const std::string& method, const std::string& url,
                                  const std::string& body,
                                  const std::map<std::string, std::string>& headers) {
    HttpResponse response;

    CURL* curl = curl_easy_init();
    if (!curl) {
        response.error = "Failed to initialize CURL";
        return response;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, m_timeout);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response.body);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, headerCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response.headers);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    if (!m_basicAuth.empty()) {
        curl_easy_setopt(curl, CURLOPT_USERPWD, m_basicAuth.c_str());
    }

    // Set method
    if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());
    } else if (method == "PUT") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());
    } else if (method == "DELETE") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }

    // Set headers
    struct curl_slist* headerList = nullptr;
    for (auto& [key, val] : headers) {
        std::string header = key + ": " + val;
        headerList = curl_slist_append(headerList, header.c_str());
    }
    if (headerList) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
    }

    CURLcode res = curl_easy_perform(curl);

    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.statusCode);
        response.success = (response.statusCode >= 200 && response.statusCode < 300);
    } else {
        response.error = curl_easy_strerror(res);
    }

    if (headerList) curl_slist_free_all(headerList);
    curl_easy_cleanup(curl);

    return response;
}

HttpResponse HttpClient::get(const std::string& url, const std::map<std::string, std::string>& headers) {
    return perform("GET", url, "", headers);
}

HttpResponse HttpClient::post(const std::string& url, const std::string& body,
                               const std::map<std::string, std::string>& headers) {
    return perform("POST", url, body, headers);
}

HttpResponse HttpClient::put(const std::string& url, const std::string& body,
                              const std::map<std::string, std::string>& headers) {
    return perform("PUT", url, body, headers);
}

HttpResponse HttpClient::del(const std::string& url, const std::map<std::string, std::string>& headers) {
    return perform("DELETE", url, "", headers);
}

void HttpClient::setTimeout(int seconds) {
    m_timeout = seconds;
}

void HttpClient::setBasicAuth(const std::string& username, const std::string& password) {
    m_basicAuth = username + ":" + password;
}

} // namespace PSM

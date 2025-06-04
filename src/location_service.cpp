#include "../include/location_service.h"
#include <windows.h>
#include <wininet.h>
#include <string>
#include <vector>
#include <sstream> // For std::ostringstream

// Link with wininet.lib
#pragma comment(lib, "wininet.lib")

namespace LocationService {

// Helper function to perform HTTP GET request using WinINet
std::string http_get(const std::wstring& url) {
    HINTERNET hInternet = NULL;
    HINTERNET hConnect = NULL;
    std::string response_data;

    hInternet = InternetOpenW(
        L"TinyShellLocationClient", // User agent
        INTERNET_OPEN_TYPE_DIRECT,  // Access type
        NULL,                       // Proxy name
        NULL,                       // Proxy bypass
        0                           // Flags
    );

    if (hInternet == NULL) {
        return "Error: InternetOpenW failed (" + std::to_string(GetLastError()) + ")";
    }

    // Parse URL to get hostname and path
    URL_COMPONENTSW urlComp;
    wchar_t szHostName[256] = {0};
    wchar_t szUrlPath[2048] = {0};

    ZeroMemory(&urlComp, sizeof(urlComp));
    urlComp.dwStructSize = sizeof(urlComp);
    urlComp.lpszHostName = szHostName;
    urlComp.dwHostNameLength = ARRAYSIZE(szHostName);
    urlComp.lpszUrlPath = szUrlPath;
    urlComp.dwUrlPathLength = ARRAYSIZE(szUrlPath);
    // urlComp.nScheme = INTERNET_SCHEME_HTTP; // This was okay, but not always needed for InternetOpenUrlW

    if (!InternetCrackUrlW(url.c_str(), url.length(), 0, &urlComp)) {
        InternetCloseHandle(hInternet);
        return "Error: InternetCrackUrlW failed (" + std::to_string(GetLastError()) + ")";
    }
    
    // For ip-api.com, it's HTTP, not HTTPS, by default for the free tier.
    // If using a service that requires HTTPS, port would be INTERNET_DEFAULT_HTTPS_PORT and flags INTERNET_FLAG_SECURE
    hConnect = InternetOpenUrlW(
        hInternet,
        url.c_str(),
        NULL, // Headers
        0,    // Headers length
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE,
        0     // Context
    );

    if (hConnect == NULL) {
        InternetCloseHandle(hInternet);
        return "Error: InternetOpenUrlW failed for " + std::string(url.begin(), url.end()) + " (" + std::to_string(GetLastError()) + ")";
    }

    char buffer[4096];
    DWORD bytesRead = 0;
    while (InternetReadFile(hConnect, buffer, sizeof(buffer) -1, &bytesRead) && bytesRead > 0) {
        buffer[bytesRead] = '\0'; // Null-terminate the buffer
        response_data.append(buffer);
    }

    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    if (response_data.empty() && GetLastError() != ERROR_SUCCESS && GetLastError() != ERROR_INTERNET_NAME_NOT_RESOLVED /* if bytesRead was 0 initially */ ) {
         if (bytesRead == 0 && response_data.empty()){ // Success but no data (e.g. empty response)
            // This case might indicate an issue or just an empty body.
            // For ip-api, an empty response is unlikely on success.
         } else {
            return "Error: InternetReadFile failed or no data received (" + std::to_string(GetLastError()) + ")";
         }
    }
    return response_data;
}

// Basic JSON value extraction (very fragile - for demonstration only)
std::string parse_json_value(const std::string& json_response, const std::string& key) {
    std::string search_key = "\"" + key + "\":"; // e.g., "city":
    size_t key_pos = json_response.find(search_key);
    if (key_pos == std::string::npos) {
        return ""; // Key not found
    }

    size_t value_start_pos = key_pos + search_key.length();
    
    // Skip whitespace and find the opening quote of the value
    while (value_start_pos < json_response.length() && isspace(json_response[value_start_pos])) {
        value_start_pos++;
    }
    if (value_start_pos >= json_response.length() || json_response[value_start_pos] != '\"') {
        return ""; // Expected a string value starting with "
    }
    value_start_pos++; // Move past the opening quote

    size_t value_end_pos = json_response.find('\"', value_start_pos);
    if (value_end_pos == std::string::npos) {
        return ""; // Closing quote not found
    }

    return json_response.substr(value_start_pos, value_end_pos - value_start_pos);
}


std::string get_current_location_placeholder() {
    // Using ip-api.com as a free geolocation API
    // It's important to review their terms of service for any usage limitations.
    std::wstring api_url = L"http://ip-api.com/json/"; 
    std::string json_response = http_get(api_url);

    if (json_response.rfind("Error:", 0) == 0) { // Check if response starts with "Error:"
        return json_response; // Return the error message from http_get
    }
    
    if (json_response.empty()) {
        return "Error: No response from geolocation service.";
    }

    // Basic parsing (highly fragile)
    std::string status = parse_json_value(json_response, "status");
    if (status != "success") {
        std::string message = parse_json_value(json_response, "message");
        return "Geolocation API Error: " + (message.empty() ? "Failed to get location details. Status: " + status : message);
    }

    std::string city = parse_json_value(json_response, "city");
    std::string regionName = parse_json_value(json_response, "regionName");
    std::string country = parse_json_value(json_response, "country");
    std::string isp = parse_json_value(json_response, "isp");
    std::string query = parse_json_value(json_response, "query"); // Your IP

    std::ostringstream oss;
    oss << "Current Location (IP-based):\n";
    if (!city.empty()) oss << "  City: " << city << "\n";
    if (!regionName.empty()) oss << "  Region: " << regionName << "\n";
    if (!country.empty()) oss << "  Country: " << country << "\n";
    if (!isp.empty()) oss << "  ISP: " << isp << "\n";
    if (!query.empty()) oss << "  IP Address: " << query << "\n";
    
    if (city.empty() && country.empty()) {
        return "Location details not found in API response.\nFull response for debugging:\n" + json_response;
    }

    return oss.str();
}

} // namespace LocationService

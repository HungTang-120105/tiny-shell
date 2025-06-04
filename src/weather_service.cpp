#include "../include/weather_service.h"
#include <windows.h>
#include <wininet.h>
#include <string>
#include <vector>
#include <sstream> // For std::ostringstream

// Include nlohmann/json - Ensure this path is correct based on where you placed json.hpp
#include "nlohmann/json.hpp" 

// Link with wininet.lib
#pragma comment(lib, "wininet.lib")

// For convenience with nlohmann/json
using json = nlohmann::json;

namespace WeatherService {

// Replicated Helper function to perform HTTP GET request using WinINet
// In a real project, this would be in a shared utility module.
std::string http_get_weather(const std::wstring& url) {
    HINTERNET hInternet = NULL;
    HINTERNET hConnect = NULL;
    std::string response_data;

    hInternet = InternetOpenW(
        L"TinyShellWeatherClient",   // User agent
        INTERNET_OPEN_TYPE_DIRECT,  // Access type
        NULL,                       // Proxy name
        NULL,                       // Proxy bypass
        0                           // Flags
    );

    if (hInternet == NULL) {
        return "Error: InternetOpenW failed (" + std::to_string(GetLastError()) + ")";
    }

    // For OpenWeatherMap, URLs are typically HTTPS.
    // InternetOpenUrl should handle HTTPS if the URL specifies it.
    // For more robust HTTPS, one might use INTERNET_FLAG_SECURE, but it depends on the API and server.
    hConnect = InternetOpenUrlW(
        hInternet,
        url.c_str(),
        NULL, // Headers
        0,    // Headers length
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE,
        0     // Context
    );

    if (hConnect == NULL) {
        InternetCloseHandle(hInternet);
        std::string error_detail = "Error: InternetOpenUrlW failed for " + 
                                 std::string(url.begin(), url.end()) + 
                                 " (" + std::to_string(GetLastError()) + ")";
        // Check for common HTTPS/SSL errors if possible, though GetLastError() is general
        if (GetLastError() == ERROR_INTERNET_INVALID_CA || GetLastError() == ERROR_INTERNET_SEC_CERT_CN_INVALID) {
            error_detail += ". This might be an SSL certificate issue.";
        }
        return error_detail;
    }

    char buffer[8192]; // Increased buffer for potentially larger weather JSON
    DWORD bytesRead = 0;
    while (InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
        buffer[bytesRead] = '\0'; // Null-terminate the buffer
        response_data.append(buffer);
    }

    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    if (response_data.empty() && GetLastError() != ERROR_SUCCESS && GetLastError() != ERROR_INTERNET_NAME_NOT_RESOLVED) {
         if (bytesRead == 0 && response_data.empty()){ 
             // No actual data read, but no error from InternetReadFile in the loop.
             // This could mean an empty response body from the server.
         } else {
            return "Error: InternetReadFile failed or no data received (" + std::to_string(GetLastError()) + ")";
         }
    }
    return response_data;
}

std::string get_weather_placeholder(const std::string& city_name_or_coords) {
    std::string api_key = "75eeb01241e1b90da86ef3014bd0e458"; // Your actual API key
    // No need for the "YOUR_API_KEY" check if you've put your real key above.

    std::wostringstream oss_url;
    oss_url << L"https://api.openweathermap.org/data/2.5/weather?q=" 
            << std::wstring(city_name_or_coords.begin(), city_name_or_coords.end()) 
            << L"&appid=" << std::wstring(api_key.begin(), api_key.end()) 
            << L"&units=metric";

    std::wstring api_url = oss_url.str();
    std::string http_response_str = http_get_weather(api_url);

    if (http_response_str.rfind("Error:", 0) == 0) { 
        return http_response_str; 
    }
    if (http_response_str.empty()) {
        return "Error: No response from weather service.";
    }

    try {
        json data = json::parse(http_response_str);

        // Check for API errors indicated by 'cod' field (e.g. 401, 404)
        // OWM returns cod as an int (e.g. 200) or string (e.g. "404") sometimes.
        // Let's check if it exists first.
        if (!data.contains("cod")) {
            return "Weather API Error: 'cod' field missing in response.\nFull response: " + http_response_str;
        }
        
        // Handle cases where 'cod' might be a string or number
        std::string cod_str;
        if (data["cod"].is_string()) {
            cod_str = data["cod"].get<std::string>();
        } else if (data["cod"].is_number()) { // More general check for number
            cod_str = std::to_string(data["cod"].get<long long>()); // Use long long for safety
        } else {
             return "Weather API Error: 'cod' field has unexpected type ('" + std::string(data["cod"].type_name()) + "').\nFull response: " + http_response_str;
        }

        if (cod_str == "401") {
            std::string message = data.value("message", "Invalid API key or unauthorized.");
            return "Weather API Error (401): " + message + "\nFull response: " + http_response_str;
        }
        if (cod_str != "200") {
            std::string message = data.value("message", "Could not fetch weather.");
            return "Weather API Error (cod: " + cod_str + "): " + message + "\nFull response: " + http_response_str;
        }

        // Safely access nested values using .value() with a default or checking with .contains()
        std::string city_found = data.value("name", "N/A");
        double temp = -999.0;
        if (data.contains("main") && data["main"].is_object() && data["main"].contains("temp") && data["main"]["temp"].is_number()){
            temp = data["main"]["temp"].get<double>();
        }
        std::string description = "N/A";
        if (data.contains("weather") && data["weather"].is_array() && !data["weather"].empty()) {
            const auto& first_weather_entry = data["weather"][0];
            if (first_weather_entry.is_object() && first_weather_entry.contains("description") && first_weather_entry["description"].is_string()){
                 description = first_weather_entry["description"].get<std::string>();
            }
        }
        double humidity = -1.0;
        if (data.contains("main") && data["main"].is_object() && data["main"].contains("humidity") && data["main"]["humidity"].is_number()){
            humidity = data["main"]["humidity"].get<double>();
        }
        double wind_speed = -1.0;
        if (data.contains("wind") && data["wind"].is_object() && data["wind"].contains("speed") && data["wind"]["speed"].is_number()){
            wind_speed = data["wind"]["speed"].get<double>();
        }

        if (city_found == "N/A" || temp == -999.0 || description == "N/A") {
             std::ostringstream error_details_oss;
             error_details_oss << "Weather data parsing failed. Some essential fields were not found or had unexpected types.\n"
                               << "Parsed values: City='" << city_found << "', Temp=" << temp 
                               << ", Desc='" << description << "'.\nFull response for debugging:\n" 
                               << http_response_str;
             return error_details_oss.str();
        }

        std::ostringstream oss_weather;
        oss_weather.precision(1); // For one decimal place for temp, etc.
        oss_weather << std::fixed;
        oss_weather << "Weather in " << city_found << ":\n"
                    << "  Temperature: " << temp << " C\n"
                    << "  Conditions:  " << description << "\n";
        if (humidity != -1.0) {
            oss_weather << "  Humidity:    " << humidity << "%\n";
        }
        if (wind_speed != -1.0) {
            oss_weather << "  Wind Speed:  " << wind_speed << " m/s\n";
        }
        return oss_weather.str();

    } catch (json::parse_error& e) {
        return "JSON Parse Error: " + std::string(e.what()) + "\nLocation in JSON (approx byte): " + std::to_string(e.byte) + "\nOriginal response:\n" + http_response_str;
    } catch (json::type_error& e) {
        return "JSON Type Error: " + std::string(e.what()) + "\nOriginal response:\n" + http_response_str;
    } catch (json::exception& e) { // Catch any other nlohmann::json exceptions
        return "JSON General Library Error: " + std::string(e.what()) + "\nOriginal response:\n" + http_response_str;
    }
    return "An unexpected error occurred during weather processing. Check logs or response string.\nFull response: " + http_response_str; // Fallback
}

} // namespace WeatherService 
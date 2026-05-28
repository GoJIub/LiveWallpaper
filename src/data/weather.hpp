#ifndef WEATHER_HPP
#define WEATHER_HPP

#include <chrono>
#include <string>

#include <curl/curl.h>

struct WeatherData {
    double temperature = 20.0;
    double wind_speed = 0.0;
    double wind_direction = 0.0;
};

class WeatherClient {
public:
    WeatherClient();
    ~WeatherClient();

    bool update();
    WeatherData current() const;

private:
    void start_request();
    bool finish_request(CURLcode result);
    void cleanup_request();

    CURLM* multi = nullptr;
    CURL* easy = nullptr;
    bool easy_added = false;
    std::string response;
    std::string request_url;
    char error_buffer[CURL_ERROR_SIZE] = {};
    WeatherData weather_data;
    std::chrono::steady_clock::time_point last_request;
};

#endif // WEATHER_HPP

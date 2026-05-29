#include "weather.hpp"

#include <nlohmann/json.hpp>

#include <chrono>
#include <iostream>
#include <mutex>
#include <stdexcept>


static void init_curl() {
    static std::once_flag init_flag;
    static CURLcode init_result = CURLE_OK;

    std::call_once(init_flag, []() {
        init_result = curl_global_init(CURL_GLOBAL_DEFAULT);
    });

    if (init_result != CURLE_OK) {
        throw std::runtime_error("Failed to initialize CURL globally");
    }
}

static size_t write_response(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* response = static_cast<std::string*>(userdata);
    response->append(ptr, size * nmemb);
    return size * nmemb;
}

std::string WeatherClient::weather_url() const {
    return "https://api.open-meteo.com/v1/forecast?latitude=" +
        std::to_string(latitude) +
        "&longitude=" +
        std::to_string(longitude) +
        "&current_weather=true";
}

WeatherClient::WeatherClient(double latitude, double longitude, int update_interval) :
    latitude(latitude),
    longitude(longitude),
    update_interval(std::chrono::minutes(update_interval))
{
    init_curl();

    multi = curl_multi_init();
    if (!multi) {
        throw std::runtime_error("Failed to initialize CURL multi handle");
    }

    last_request = std::chrono::steady_clock::now() - this->update_interval;
}

WeatherClient::~WeatherClient() {
    cleanup_request();
    if (multi) {
        curl_multi_cleanup(multi);
    }
}

bool WeatherClient::update() {
    auto now = std::chrono::steady_clock::now();
    if (!easy && now - last_request >= update_interval) {
        start_request();
        last_request = now;
    }

    if (!easy) {
        return false;
    }

    int running_handles = 0;
    CURLMcode multi_result = curl_multi_perform(multi, &running_handles);
    if (multi_result != CURLM_OK) {
        std::cerr << "Weather update error: " << curl_multi_strerror(multi_result) << std::endl;
        cleanup_request();
        return false;
    }

    bool updated = false;
    int messages_left = 0;
    while (CURLMsg* message = curl_multi_info_read(multi, &messages_left)) {
        if (message->msg == CURLMSG_DONE && message->easy_handle == easy) {
            updated = finish_request(message->data.result) || updated;
        }
    }

    return updated;
}

WeatherData WeatherClient::current() const {
    return weather_data;
}

void WeatherClient::start_request() {
    easy = curl_easy_init();
    if (!easy) {
        std::cerr << "Weather update error: failed to initialize CURL request" << std::endl;
        return;
    }

    response.clear();
    request_url = weather_url();
    error_buffer[0] = '\0';

    curl_easy_setopt(easy, CURLOPT_URL, request_url.c_str());
    curl_easy_setopt(easy, CURLOPT_ERRORBUFFER, error_buffer);
    curl_easy_setopt(easy, CURLOPT_USERAGENT, "LiveWallpaper/1.0");
    curl_easy_setopt(easy, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(easy, CURLOPT_CONNECTTIMEOUT_MS, 2000L);
    curl_easy_setopt(easy, CURLOPT_TIMEOUT_MS, 5000L);
    curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, write_response);
    curl_easy_setopt(easy, CURLOPT_WRITEDATA, &response);

    CURLMcode multi_result = curl_multi_add_handle(multi, easy);
    if (multi_result != CURLM_OK) {
        std::cerr << "Weather update error: " << curl_multi_strerror(multi_result) << std::endl;
        cleanup_request();
        return;
    }
    easy_added = true;
}

bool WeatherClient::finish_request(CURLcode result) {
    if (result != CURLE_OK) {
        std::string error = error_buffer[0] != '\0'
            ? error_buffer
            : curl_easy_strerror(result);
        std::cerr << "Weather update error: " << error << std::endl;
        cleanup_request();
        return false;
    }

    long response_code = 0;
    curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code < 200 || response_code >= 300) {
        std::cerr << "Weather update error: HTTP " << response_code << std::endl;
        cleanup_request();
        return false;
    }

    bool updated = false;
    try {
        auto root = nlohmann::json::parse(response);
        const auto& current_weather = root.at("current_weather");
        weather_data.temperature = current_weather.at("temperature").get<double>();
        weather_data.wind_speed = current_weather.at("windspeed").get<double>();
        weather_data.wind_direction = current_weather.at("winddirection").get<double>();
        updated = true;
    } catch (const std::exception& e) {
        std::cerr << "Weather update error: " << e.what() << std::endl;
    }

    cleanup_request();
    return updated;
}

void WeatherClient::cleanup_request() {
    if (!easy) {
        return;
    }

    if (easy_added) {
        curl_multi_remove_handle(multi, easy);
    }
    curl_easy_cleanup(easy);
    easy = nullptr;
    easy_added = false;
}

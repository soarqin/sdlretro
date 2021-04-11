#pragma once

#include <functional>
#include <string>
#include <map>
#include <queue>

namespace util {

class Downloader final {
public:
    struct Data {
        int uid;
        long response_code;
        std::string url;
        std::string effective_url;
        std::string content;
    };

public:
    Downloader();
    ~Downloader();

    int add(const std::string &url, std::function<void(int, int64_t, int64_t)> = nullptr);

    /* pass current tick in usec, return time to wait till next call */
    uint64_t process(uint64_t usec);

    inline int get_running() const { return still_running; }
    inline bool pop_response(Data &data) {
        if (responses.empty()) {
            return false;
        }
        data = std::move(responses.front());
        responses.pop();
        return true;
    }

private:
    void *multi_handle;
    std::map<int, struct Downloading> downloads;
    std::queue<Data> responses;
    int still_running = 0;
    uint64_t timeout_us = 0;
};

}

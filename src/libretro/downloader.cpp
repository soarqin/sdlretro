#include "downloader.h"

#include <curl/curl.h>

namespace util {

struct Downloading {
    std::function<void(int, int64_t, int64_t)> progress_callback;
    int64_t dlnow = -1, dltotal = -1;
    Downloader::Data data;
};

static int uid_max = 0;

Downloader::Downloader() {
    curl_global_init(CURL_GLOBAL_ALL);
    multi_handle = curl_multi_init();
}

Downloader::~Downloader() {
    curl_multi_cleanup(multi_handle);
    multi_handle = nullptr;
    curl_global_cleanup();
}

static size_t write_func(char *ptr, size_t size, size_t nmemb, void *userdata) {
    auto *d = static_cast<Downloading*>(userdata);
    d->data.content.append(ptr, size * nmemb);
    return size * nmemb;
}

static int prog_func(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    auto *d = static_cast<Downloading*>(clientp);
    if (d->progress_callback) {
        if (d->dlnow == dlnow || d->dltotal == dltotal) {
            return 0;
        }
        d->dlnow = dlnow;
        d->dltotal = dltotal;
        d->progress_callback(d->data.uid, dlnow, dltotal);
    }
    return 0;
}

int Downloader::add(const std::string &url, std::function<void(int, int64_t, int64_t)> prog_cb) {
    auto *c = curl_easy_init();
    if (!c) {
        return -1;
    }
    if (curl_multi_add_handle(multi_handle, c) != CURLM_OK) {
        curl_easy_cleanup(c);
        return -1;
    }
    auto &d = downloads[++uid_max];
    d.progress_callback = std::move(prog_cb);
    d.data.uid = uid_max;
    d.data.url = url;
    curl_easy_setopt(c, CURLOPT_URL, url.c_str());
    curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(c, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(c, CURLOPT_IGNORE_CONTENT_LENGTH, 0L);
    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, write_func);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, &d);
    curl_easy_setopt(c, CURLOPT_XFERINFOFUNCTION, prog_func);
    curl_easy_setopt(c, CURLOPT_XFERINFODATA, &d);
    curl_easy_setopt(c, CURLOPT_PRIVATE, &d);
    curl_multi_perform(multi_handle, &still_running);
    return uid_max;
}

uint64_t Downloader::process(uint64_t usec) {
    if (timeout_us > usec) {
        return timeout_us - usec;
    }

    curl_multi_perform(multi_handle, &still_running);

    while (true) {
        int msg_in_queue;
        auto *msg = curl_multi_info_read(multi_handle, &msg_in_queue);
        if (!msg) {
            break;
        }
        if (msg->msg == CURLMSG_DONE) {
            curl_multi_remove_handle(multi_handle, msg->easy_handle);
            Downloading *d;
            curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &d);
            auto ite = downloads.find(d->data.uid);
            if (ite != downloads.end()) {
                auto &data = ite->second.data;
                char *url = nullptr;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &data.response_code);
                curl_easy_getinfo(msg->easy_handle, CURLINFO_EFFECTIVE_URL, &url);
                data.effective_url = url;
                responses.emplace(std::move(data));
                downloads.erase(ite);
            }
            curl_easy_cleanup(msg->easy_handle);
        }
    }

    if (still_running) {
        long timeout;
        auto code = curl_multi_timeout(multi_handle, &timeout);
        if (code == CURLM_OK && timeout > 0) {
            if (timeout > 500) {
                timeout = 500;
            }
            uint64_t timeout_in_us = 1000ULL * static_cast<uint64_t>(timeout);
            timeout_us = usec + timeout_in_us;
            return timeout_in_us;
        }
    }
    return 0ULL;
}

}

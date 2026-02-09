// Logger.hpp
#pragma once
#include <curl/curl.h>
#include <string>
#include <atomic>
#include <chrono>

namespace NetLogger
{

    struct IoLog
    {
        std::atomic<uint64_t> first_byte_in_us{0};
        std::atomic<uint64_t> first_byte_out_us{0};

        std::string req_headers;  // collects outgoing request headers
        std::string resp_headers; // collects incoming response headers
    };

    inline uint64_t now_us()
    {
        return (uint64_t)std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::steady_clock::now().time_since_epoch())
            .count();
    }

    // Write callback that logs first incoming byte and appends data
    inline std::size_t logging_write_cb(const char *in, std::size_t size, std::size_t num, void *userdata)
    {
        const std::size_t totalBytes = size * num;
        auto *bundle = static_cast<std::pair<std::string *, IoLog *> *>(userdata);
        auto *out = bundle->first;
        auto *io = bundle->second;

        const uint64_t t = now_us();
        if (totalBytes > 0 && io->first_byte_in_us.load(std::memory_order_relaxed) == 0)
        {
            io->first_byte_in_us.store(t, std::memory_order_relaxed);
            // You can replace this with your logging system
            // fprintf(stderr, "First RX byte @ %llu us\n", (unsigned long long)t);
        }

        out->append(in, totalBytes);
        return totalBytes;
    }

    // libcurl debug callback for tracking first send/receive bytes at socket level
    inline int curl_debug_cb(CURL *, curl_infotype type, char *data, size_t size, void *userptr)
    {
        auto *io = static_cast<IoLog *>(userptr);
        const uint64_t t = now_us();

        switch (type)
        {
        case CURLINFO_HEADER_OUT:
            if (size > 0 && io->first_byte_out_us.load(std::memory_order_relaxed) == 0)
            {
                io->first_byte_out_us.store(t, std::memory_order_relaxed);
            }
            io->req_headers.append(data, size);
            // print exactly what was sent
            //fwrite(data, 1, size, stderr);
            break;

        case CURLINFO_DATA_OUT:
            if (size > 0 && io->first_byte_out_us.load(std::memory_order_relaxed) == 0)
            {
                io->first_byte_out_us.store(t, std::memory_order_relaxed);
                // fprintf(stderr, "First TX byte @ %llu us\n", (unsigned long long)t);
            }
            break;
        case CURLINFO_HEADER_IN:
            io->resp_headers.append(data, size);
            // print exactly what was received (headers)
            //fwrite(data, 1, size, stderr);
            break;
        case CURLINFO_DATA_IN:
            // Optional: log every inbound chunk
            // fprintf(stderr, "RX %zu bytes @ %llu us\n", size, (unsigned long long)t);
            break;
        default:
            break;
        }
        return 0;
    }

} // namespace NetLogger

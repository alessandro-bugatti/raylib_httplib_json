// net_wrapper.hpp
#pragma once
#include <thread>
#include <queue>
#include <mutex>
#include <optional>
#include <condition_variable>
#include <chrono>

#include "httplib.h"
#include "json.hpp"

namespace net {

using json = nlohmann::json;

// ---- struttura richieste e risposte ----
struct Request {
    enum Type { GET, SET } type;
    std::string key;
    std::string value;   // usato solo per SET
};

struct Response {
    std::string key;
    std::optional<std::string> value; // nullopt → errore
};

// ---- coda thread-safe ----
std::queue<Request> reqQueue;
std::queue<Response> resQueue;
std::mutex reqMutex, resMutex;

// ---- stato client ----
std::string currentServerUrl;
std::mutex stateMutex;
bool isInitialized = false;

// ---- thread di networking ----
bool running = true;

void worker(std::string serverUrl) {
    httplib::Client cli(serverUrl);

    while (running) {
        Request req;
        {
            std::unique_lock<std::mutex> lock(reqMutex);
            if (reqQueue.empty()) {
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                continue;
            }
            req = reqQueue.front();
            reqQueue.pop();
        }

        if (req.type == Request::GET) {
            auto res = cli.Get(("/get/" + req.key).c_str());

            Response out;
            out.key = req.key;

            if (res && res->status == 200) {
                out.value = res->body;
            } else {
                out.value = std::nullopt;
            }

            std::lock_guard<std::mutex> lock(resMutex);
            resQueue.push(out);

        } else if (req.type == Request::SET) {
            json j;
            j["value"] = req.value;

            auto res = cli.Post(
                ("/set/" + req.key).c_str(),
                j.dump(),
                "application/json"
            );

            // Invia risposta anche se non serve
            Response out;
            out.key = req.key;
            out.value = (res && res->status == 200) ? std::make_optional(req.value) : std::nullopt;

            std::lock_guard<std::mutex> lock(resMutex);
            resQueue.push(out);
        }
    }
}

// ---- API per studenti ----
void init(const std::string &serverUrl) {
    {
        std::lock_guard<std::mutex> lock(stateMutex);
        currentServerUrl = serverUrl;
        isInitialized = true;
    }

    static std::thread t(worker, serverUrl);
    t.detach();
}

bool ping(int timeoutMs = 300) {
    std::string serverUrl;
    {
        std::lock_guard<std::mutex> lock(stateMutex);
        if (!isInitialized || currentServerUrl.empty()) {
            return false;
        }
        serverUrl = currentServerUrl;
    }

    httplib::Client cli(serverUrl);
    cli.set_connection_timeout(std::chrono::milliseconds(timeoutMs));
    cli.set_read_timeout(std::chrono::milliseconds(timeoutMs));
    cli.set_write_timeout(std::chrono::milliseconds(timeoutMs));

    auto res = cli.Get("/get/__ping__");
    return static_cast<bool>(res);
}

void get(const std::string& key) {
    std::lock_guard<std::mutex> lock(reqMutex);
    reqQueue.push(Request{Request::GET, key, ""});
}

void set(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(reqMutex);
    reqQueue.push(Request{Request::SET, key, value});
}

// Main loop: prova a leggere una risposta
bool pollResponse(Response& out) {
    std::lock_guard<std::mutex> lock(resMutex);
    if (resQueue.empty()) return false;

    out = resQueue.front();
    resQueue.pop();
    return true;
}

} // namespace net

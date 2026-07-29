// Self-evaluation: modern C++ competency sample
// Build (example): g++ -std=c++20 -Wall -Wextra -Werror sample.cpp -pthread
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>

#include <utility>
#include <vector>
// ---- Const vs constexpr ----
constexpr std::size_t kMaxRecent = 8;
constexpr bool is_power_of_two(std::size_t v) noexcept {
    return v && ((v & (v - 1)) == 0);
}
static_assert(is_power_of_two(8));
static_assert(kMaxRecent == 8);
// ---- RAII + locking ----
class ScopedTimer {
public:
    using Clock = std::chrono::steady_clock;
    explicit ScopedTimer(std::string_view label)
    : label_(label), start_(Clock::now()) {}
    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;
    ~ScopedTimer() noexcept {
        const auto end = Clock::now();
        const auto us = std::chrono::duration_cast<std::chrono::microseconds>(end -
        start_).count();
        std::cout << label_ << " took " << us << " us\n";
    }
private:
    std::string label_;
    Clock::time_point start_;
};
// ---- OOP with virtual interface ----
struct IEventSink {
    virtual ~IEventSink() = default;
    virtual void on_event(std::string_view key, int value) = 0;
};

class StdoutSink final : public IEventSink {
public:
    void on_event(std::string_view key, int value) override {
        std::cout << "event: " << key << " -> " << value << "\n";
    }
};
// ---- Template: "producing new types" + iterator/algorithm usage ----
template <typename T, typename KeyFn>
class RecentByKey {
public:
    using value_type = T;
    explicit RecentByKey(KeyFn key_fn)
    : key_fn_(std::move(key_fn)) {}
    void push(T v) {
        // We want to keep only unique-by-key items, most recent first, size <= kMaxRecent.
        // This is intentionally written with algorithms/iterators rather than raw loops.
        auto key = key_fn_(v);
        // erase-remove idiom using a lambda (captures "this" and "key")
        items_.erase(
            std::remove_if(items_.begin(), items_.end(),
                           [&](const T& item) { return key_fn_(item) == key; }),
                     items_.end());
        items_.insert(items_.begin(), std::move(v));
        if (items_.size() > kMaxRecent) {
            items_.resize(kMaxRecent);
        }
    }
    // Return optional reference wrapper (avoid pointer ownership confusion)
    std::optional<std::reference_wrapper<const T>> find_by_key(std::invoke_result_t<KeyFn, const T&> key) const
    {
            auto it = std::find_if(items_.begin(), items_.end(),

                                   [&](const T& item) { return key_fn_(item) == key; });

            if (it == items_.end()) return std::nullopt;
            return std::cref(*it);
    }
    const std::vector<T>& items() const noexcept { return items_; }
private:
    KeyFn key_fn_; // note: template parameter makes a concrete type here
    std::vector<T> items_;
};
// ---- Thread-safe store using RAII locks ----
class EventStore {
public:
    explicit EventStore(std::shared_ptr<IEventSink> sink)
    : sink_(std::move(sink)) {}
    void record(std::string key, int value) {
        // lock scope is explicit and exception-safe
        {
            std::unique_lock lock(mu_);
            data_[key] = value;
            recent_.push({key, value});
        }
        // call sink without holding lock (avoid re-entrancy/deadlock)
        sink_->on_event(key, value);
    }
    std::optional<int> get(std::string_view key) const {
        std::shared_lock lock(mu_);
        auto it = data_.find(std::string(key));
        if (it == data_.end()) return std::nullopt;
        return it->second;
    }
    std::vector<std::pair<std::string, int>> recent() const {
        std::shared_lock lock(mu_);
        return recent_.items(); // returns copy; caller owns it
    }
private:

    mutable std::shared_mutex mu_;
    std::unordered_map<std::string, int> data_;
    using Pair = std::pair<std::string, int>;
    RecentByKey<Pair, std::function<std::string_view(const Pair&)>> recent_{
        [](const Pair& p) -> std::string_view { return p.first; }
    };
    std::shared_ptr<IEventSink> sink_;
};
// ---- Demonstration driver ----
int main() {
    ScopedTimer timer("demo");
    auto sink = std::make_shared<StdoutSink>();
    EventStore store(sink);
    std::atomic<bool> done{false};
    std::thread writer([&] {
        for (int i = 0; i < 20; ++i) {
            store.record("k" + std::to_string(i % 5), i);
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        done = true;
    });
    std::thread reader([&] {
        while (!done.load()) {
            auto v = store.get("k1");
            if (v) {
                // "if (v)" relies on optional<bool> conversion rules
                // (optional has operator bool, not the contained int)
                std::cout << "read k1 = " << *v << "\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
    });
    writer.join();

    reader.join();
    // recent keys printed in most-recent-first order, unique by key
    for (const auto& [k, v] : store.recent()) {
        std::cout << "recent: " << k << " -> " << v << "\n";
    }
    return 0;
}


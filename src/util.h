#pragma once

#include <atomic>
#include <mutex>

class GlobalSeedGenerator {
private:
    std::atomic<uint64_t> seed;
    std::mutex mutex;

    GlobalSeedGenerator() : seed(0) {}

public:
    static uint64_t get_next_seed() {
        return get_instance().get_next_seed_internal();
    }

    static void initialize(uint64_t initial_seed = 0) {
        get_instance().seed.store(initial_seed, std::memory_order_seq_cst);
    }

    static GlobalSeedGenerator& get_instance() {
        static GlobalSeedGenerator instance;
        return instance;
    }

private:
    uint64_t get_next_seed_internal() {
        std::lock_guard<std::mutex> lock(mutex);
        return seed.fetch_add(1, std::memory_order_seq_cst);
    }
};
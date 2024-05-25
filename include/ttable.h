#pragma once
#include <array>
#include "move.h"

struct TTEntry_perft {
    uint64_t key = 0;
    uint64_t node_count = 0;
    int depth_searched = 0;
};

template <typename Entry, size_t MB>
class TTable {
    static constexpr size_t _size = (MB * 1000 * 1000) / sizeof(Entry);
    std::array<Entry, _size> table;

public:
    TTable() = default;
    ~TTable() = default;

    template <typename... Args>
    inline void emplace(uint64_t key, Args&&... args)
    {
        const uint64_t index = getIdx(key);
        table[index] = Entry { key, std::forward<Args>(args)... };
    }

    inline bool if_has_get(uint64_t key, int depth, uint64_t& nodes)
    {
        const uint64_t index = getIdx(key);
        const auto entry = table[index];
        if ( entry.key == key && entry.depth_searched == depth ) {
            nodes = entry.node_count;
            return true;
        }
        else {
            return false;
        }
    }

    inline bool has(uint64_t key, int depth) const
    {
        const uint64_t index = getIdx(key);
        const auto& entry = table[index];
        return entry.key == key && entry.depth_searched == depth;
    }

    inline uint64_t get(uint64_t key, int depth)
    {
        uint64_t index = getIdx(key);
        auto& entry = table[index];
        if ( entry.key == key && entry.depth_searched == depth ) {
            return entry.node_count;
        }
        return NULL_BB;
    }

    constexpr size_t size() const { return _size; }
private:
    inline uint64_t getIdx(uint64_t key) const { return key % _size; }
};

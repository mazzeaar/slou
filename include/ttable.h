#pragma once
#include <array>
#include "move.h"

struct TTEntry_perft {
    uint64_t key = 0;
    uint64_t node_count = 0;
    int depth_searched = 0;
};

struct TTEntry_eval {
    uint64_t key = 0;
    int depth_searched = 0;
    double best_score = 0.0;
    Move best_move = Move();
    enum { EXACT, UPPERBOUND, LOWERBOUND } type;
};

template <typename Entry, size_t MB>
class TTable {
    static constexpr size_t _size = (MB * 1000 * 1000) / sizeof(Entry);
    Entry* table;
public:
    TTable() : table(new Entry[_size]) { }
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

    inline Entry get(uint64_t key)
    {
        uint64_t index = getIdx(key);
        return table[index];
    }

    constexpr size_t size() const { return _size; }
private:
    inline uint64_t getIdx(uint64_t key) const { return key % _size; }
};

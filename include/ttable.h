#pragma once
#include <array>
#include "move.h"

struct TTableEntry {
    uint64_t key = 0;
    uint64_t node_count = 0;
    int depth_searched = 0;
    Move best_move = Move();
    double score = 0.0;
};

class TTable {
public:
    TTable(const uint64_t MB = 32)
        : _size((MB * 1024 * 1024) / sizeof(TTableEntry))
    {
        table = new TTableEntry[_size];
    }

    ~TTable() = default;

    void addEntry(uint64_t key, uint64_t node_count, int depth, const Move best_move = Move(), double score = 0.0)
    {
        uint64_t index = getIdx(key);
        table[index] = { key, node_count, depth, best_move, score };
    }

    bool has(uint64_t key, int depth) const
    {
        uint64_t index = getIdx(key);
        const auto& entry = table[index];
        return entry.key == key && entry.depth_searched == depth;
    }

    TTableEntry* get(uint64_t key, int depth)
    {
        uint64_t index = getIdx(key);
        auto& entry = table[index];
        if ( entry.key == key && entry.depth_searched == depth ) {
            return &entry;
        }
        return nullptr;
    }

    constexpr size_t size() const { return _size; }

private:
    uint64_t getIdx(uint64_t key) const { return key % _size; }
    size_t _size;
    TTableEntry* table;
};

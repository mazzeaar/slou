#pragma once
#include <array>
#include "move.h"

struct TTableEntry {
    uint64_t key;
    const Move best_move;
    double score;
    uint8_t dist_from_root;
    uint8_t depth_searched;
};

class TTable {
public:
    TTable(uint64_t mb); // mb * 1024 * 1024 = bytes. bytes / sizeof(TTableEntry) = bytes to calloc
    void addEntry();

    bool has(uint64_t key); // store the iterator we just looked up so we dont have to do it twice.
    TTableEntry* get(uint64_t key);

    constexpr size_t size() const { return _size; }
private:
    uint64_t maskKey() const;

    size_t _size;
    TTableEntry* table;
    TTableEntry* last_found_lookup;
};
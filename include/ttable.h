#pragma once
#include <unordered_map>
#include <cstdint>

class TranspositionTable {
public:
    using Key = uint64_t;
    using Value = uint64_t;

    void store(Key key, Value value)
    {
        table[key] = value;
    }

    bool lookup(Key key, Value& value) const
    {
        auto it = table.find(key);
        if ( it != table.end() ) {
            value = it->second;
            return true;
        }
        return false;
    }

    void clear()
    {
        table.clear();
    }

private:
    std::unordered_map<Key, Value> table;
};

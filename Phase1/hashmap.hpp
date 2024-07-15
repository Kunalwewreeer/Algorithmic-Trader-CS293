#ifndef HASHMAP_HPP
#define HASHMAP_HPP
#include <iostream>
#include <list>
#include <vector>
#include <string>

class hashmap
{
private:
    struct HashNode
    {
        std::string key;
        int value;
        HashNode(std::string key, int value) : key(key), value(value) {}
    };

    std::vector<std::list<HashNode>> table;
    size_t capacity;
    size_t size;

    size_t hashFunction(const std::string &key)
    {
        size_t hashValue = 0;
        for (char ch : key)
        {
            hashValue = 37 * hashValue + ch;
        }
        return hashValue % capacity;
    }

public:
    hashmap(size_t capacity = 101) : capacity(capacity), size(0)
    {
        table.resize(capacity);
    }

    void insert(const std::string &key, int value)
    {
        size_t index = hashFunction(key);
        for (auto &node : table[index])
        {
            if (node.key == key)
            {
                node.value = value;
                return;
            }
        }
        table[index].emplace_back(key, value);
        ++size;
    }

    bool remove(const std::string &key)
    {
        size_t index = hashFunction(key);
        for (auto it = table[index].begin(); it != table[index].end(); ++it)
        {
            if (it->key == key)
            {
                table[index].erase(it);
                --size;
                return true;
            }
        }
        return false;
    }

    int &operator[](const std::string &key)
    {
        size_t index = hashFunction(key);
        for (auto &node : table[index])
        {
            if (node.key == key)
            {
                return node.value;
            }
        }
        // If key not found, insert a new node with default value and return its reference
        table[index].emplace_back(key, 0);
        ++size;
        return table[index].back().value;
    }

    bool get(const std::string &key, int &value)
    {
        size_t index = hashFunction(key);
        for (const auto &node : table[index])
        {
            if (node.key == key)
            {
                value = node.value;
                return true;
            }
        }
        return false;
    }
};

#endif
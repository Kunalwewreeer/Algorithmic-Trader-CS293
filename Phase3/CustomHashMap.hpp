#ifndef CUSTOM_HASH_MAP_HPP
#define CUSTOM_HASH_MAP_HPP

#include <vector>
#include <string>

// Define the custom hash function
template <typename K>
size_t customHash(const K &key, size_t capacity)
{
    // Hash function for integers
    if constexpr (std::is_integral_v<K>)
    {
        return key % capacity;
    }
    // Hash function for strings
    else if constexpr (std::is_same_v<K, std::string>)
    {
        size_t hash = 0;
        for (char c : key)
        {
            hash = hash * 31 + c;
        }
        return hash % capacity;
    }
    // Add more cases for different types as needed
}

template <typename K, typename V>
class CustomHashMap
{
public:
    struct HashNode
    {
        K key;
        V value;
        HashNode *next;

        HashNode(K key, V value) : key(key), value(value), next(nullptr) {}
    };

private:
    std::vector<HashNode *> table;
    size_t capacity;

    size_t hash(K key)
    {
        return customHash(key, capacity); // Use the custom hash function
    }

public:
    CustomHashMap(size_t cap = 10000) : capacity(cap)
    {
        table.resize(capacity, nullptr);
    }

    ~CustomHashMap()
    {
        for (auto head : table)
        {
            while (head != nullptr)
            {
                HashNode *temp = head;
                head = head->next;
                delete temp;
            }
        }
    }

    void insert(K key, V value)
    {
        size_t index = hash(key);
        HashNode *head = table[index];

        while (head != nullptr)
        {
            if (head->key == key)
            {
                head->value = value;
                return;
            }
            head = head->next;
        }

        HashNode *newNode = new HashNode(key, value);
        newNode->next = table[index];
        table[index] = newNode;
    }

    bool remove(K key)
    {
        size_t index = hash(key);
        HashNode *head = table[index];
        HashNode *prev = nullptr;

        while (head != nullptr)
        {
            if (head->key == key)
            {
                if (prev == nullptr)
                {
                    table[index] = head->next;
                }
                else
                {
                    prev->next = head->next;
                }
                delete head;
                return true;
            }
            prev = head;
            head = head->next;
        }
        return false;
    }

    V *find(K key)
    {
        size_t index = hash(key);
        HashNode *head = table[index];

        while (head != nullptr)
        {
            if (head->key == key)
            {
                return &(head->value);
            }
            head = head->next;
        }
        return nullptr;
    }
    V &operator[](const K &key)
    {
        size_t index = hash(key);
        HashNode *head = table[index];

        // Search for the key in the chain
        while (head != nullptr)
        {
            if (head->key == key)
            {
                return head->value;
            }
            head = head->next;
        }

        // Key not found, insert a new node with default value
        V defaultValue = V();
        HashNode *newNode = new HashNode(key, defaultValue);
        newNode->next = table[index];
        table[index] = newNode;

        return newNode->value;
    }
    void clear()
    {
        for (auto &head : table)
        {
            while (head != nullptr)
            {
                HashNode *temp = head;
                head = head->next;
                delete temp;
            }
        }
        table.clear();
        table.resize(capacity, nullptr);
    }
    size_t bucketCount() const
    {
        return capacity; // Assuming 'capacity' is the number of buckets
    }
    HashNode *bucketHead(size_t index) const
    {
        return table[index]; // 'table' is the vector of HashNode pointers
    }
};

#endif // CUSTOM_HASH_MAP_HPP
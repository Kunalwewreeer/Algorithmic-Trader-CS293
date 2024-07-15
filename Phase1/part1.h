#ifndef PART1_H
#define PART1_H
#include <string>
#include <vector>
using namespace std;
struct info
{
    std::string name;
    int trade_p;
    int sell_p = 2147483647; // sell_p is when *their* action is *s* (to be maintained with the lowest value)
    int buy_p = -2147483647; // buy_p is when *their* action is *b* (to be maintained with the highest value)
    int cancelSell_p = 2147483647;
    int cancelBuy_p = -2147483647;
};

struct TrieNode
{
    TrieNode *children[52]; // to cover [a-z] and [A-Z]
    info *stockInfo;

    // Constructor
    TrieNode() : stockInfo(nullptr)
    {
        for (int i = 0; i < 52; ++i)
        {
            children[i] = nullptr;
        }
    }
};

class StockTrie
{
private:
    TrieNode *root;

    // Helper function to get index for character
    int charToIndex(char c)
    {
        if ('a' <= c && c <= 'z')
            return c - 'a';
        if ('A' <= c && c <= 'Z')
            return c - 'A' + 26; // 26 offsets for uppercase
        return -1;               // should never reach here if input is valid
    }

public:
    // Constructor
    StockTrie()
    {
        root = new TrieNode();
    }

    // Insert stock info into trie
    void insert(info *stock)
    {
        TrieNode *node = root;
        for (char c : stock->name)
        {
            int index = charToIndex(c);
            if (!node->children[index])
            {
                node->children[index] = new TrieNode();
            }
            node = node->children[index];
        }
        node->stockInfo = stock;
    }

    // Search stock based on name
    info *search(const std::string &name)
    {
        TrieNode *node = root;
        for (char c : name)
        {
            int index = charToIndex(c);
            if (!node->children[index])
            {
                return nullptr;
            }
            node = node->children[index];
        }
        return node->stockInfo;
    }
};
#endif
#include "market.h"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include "CustomHashMap.hpp"
using namespace std;
market::market(int argc, char **argv)
{
    // Constructor remains the same
}
struct info
{
    int time, price, quantity, expiry;
    string trader, action, company;
};

class InfoComparator
{
public:
    bool operator()(const info *a, const info *b) const
    {
        if (a->price != b->price)
            return a->price < b->price; // Higher price has lower priority
        if (a->time != b->time)
            return a->time > b->time; // Later time has lower priority
        return a->trader > b->trader; // Lexicographical comparison for trader
    }
};
class InfoComparator2
{
public:
    bool operator()(const info *a, const info *b) const
    {
        if (a->price != b->price)
            return a->price > b->price; // Higher price has lower priority
        if (a->time != b->time)
            return a->time > b->time; // Later time has lower priority
        return a->trader > b->trader; // Lexicographical comparison for trader
    }
};

template <typename T, typename Comparator>
class CustomPriorityQueue
{
private:
    vector<T> heap;
    Comparator comp;

    void heapifyUp(int index)
    {
        while (index > 0 && comp(heap[parent(index)], heap[index]))
        {
            swap(heap[parent(index)], heap[index]);
            index = parent(index);
        }
    }

    void heapifyDown(int index)
    {
        int smallest = index;
        int left = leftChild(index);
        int right = rightChild(index);

        if (left < size() && comp(heap[smallest], heap[left]))
        {
            smallest = left;
        }

        if (right < size() && comp(heap[smallest], heap[right]))
        {
            smallest = right;
        }

        if (smallest != index)
        {
            swap(heap[index], heap[smallest]);
            heapifyDown(smallest);
        }
    }

    int parent(int index) { return (index - 1) / 2; }
    int leftChild(int index) { return 2 * index + 1; }
    int rightChild(int index) { return 2 * index + 2; }

public:
    CustomPriorityQueue() {}

    void push(const T &value)
    {
        heap.push_back(value);
        heapifyUp(heap.size() - 1);
    }

    void pop()
    {
        if (!heap.empty())
        {
            heap[0] = heap.back();
            heap.pop_back();
            heapifyDown(0);
        }
    }

    T &top()
    {
        if (!heap.empty())
        {
            return heap[0];
        }
        throw std::out_of_range("Queue is empty");
    }

    bool empty() const
    {
        return heap.empty();
    }

    size_t size() const
    {
        return heap.size();
    }
};

struct trader_info
{
    int shares_bought = 0;
    int shares_sold = 0;
    int net_transfer = 0;
};

void market::start()
{
    ifstream inputFile("output.txt");
    CustomHashMap<string, CustomPriorityQueue<info *, InfoComparator>> buyQueue;
    CustomHashMap<string, CustomPriorityQueue<info *, InfoComparator2>> sellQueue;
    int trade_number = 0;
    int total_transfer = 0;
    int total_shares = 0;
    int transfer = 0;
    CustomHashMap<string, trader_info> trader_info_map;
    string line;
    getline(inputFile, line);
    while (getline(inputFile, line))
    {
        if (line == "!@")
            break;

        istringstream line_stream(line);
        int time, price, quantity, expiry;
        string trader, action, company;

        line_stream >> time >> trader >> action >> company;
        line_stream.ignore(2);
        line_stream >> price;
        line_stream.ignore(2);
        line_stream >> quantity;
        line_stream >> expiry;

        info *information = new info;
        information->time = time;
        information->price = price;
        information->quantity = quantity;
        information->expiry = (expiry == -1) ? 2477289 : expiry;
        expiry = (expiry == -1) ? 2477289 : expiry;
        information->trader = trader;
        information->action = action;
        information->company = company;

        if (action == "SELL")
            sellQueue[company].push(information);
        else
            buyQueue[company].push(information);

        // Order matching and expiration check
        while (!sellQueue[company].empty() && !buyQueue[company].empty())
        {
            auto seller = sellQueue[company].top();
            auto buyer = buyQueue[company].top();

            // Expired orders removal
            while (!sellQueue[company].empty() && (seller->expiry + seller->time < time))
            {
                sellQueue[company].pop();
                delete seller;
                seller = sellQueue[company].empty() ? nullptr : sellQueue[company].top();
            }

            while (!buyQueue[company].empty() && (buyer->expiry + buyer->time < time))
            {
                buyQueue[company].pop();
                delete buyer;
                buyer = buyQueue[company].empty() ? nullptr : buyQueue[company].top();
            }

            if (!seller || !buyer)
                break;

            if (buyer->price >= seller->price) // Trade occurs
            {
                int tradedQuantity = min(buyer->quantity, seller->quantity);
                int current_price;
                if (action == "SELL")
                {
                    current_price = buyer->price;
                }
                else
                {
                    current_price = seller->price;
                }
                cout << buyer->trader << " purchased " << tradedQuantity << " share of " << company << " from " << seller->trader << " for "
                     << "$" << current_price << "/share" << endl;
                trade_number++;
                transfer = current_price * tradedQuantity;
                total_transfer += transfer;
                total_shares += tradedQuantity;
                trader_info_map[seller->trader].shares_sold += tradedQuantity;
                trader_info_map[seller->trader].net_transfer += transfer;
                trader_info_map[buyer->trader].shares_bought += tradedQuantity;
                trader_info_map[buyer->trader].net_transfer -= transfer;

                if (buyer->quantity > seller->quantity)
                {
                    buyer->quantity -= tradedQuantity;
                    sellQueue[company].pop();
                    delete seller;
                }
                else if (buyer->quantity < seller->quantity)
                {
                    seller->quantity -= tradedQuantity;
                    buyQueue[company].pop();
                    delete buyer;
                }
                else // Equal quantity
                {
                    sellQueue[company].pop();
                    buyQueue[company].pop();
                    delete seller;
                    delete buyer;
                }
            }
            else
                break; // Prices don't match, so no trade occurs
        }
    }

    // Cleanup any remaining allocated memory
    // for (auto &pair : sellQueue)
    // {
    //     while (!pair.second.empty())
    //     {
    //         delete pair.second.top();
    //         pair.second.pop();
    //     }
    // }
    // for (auto &pair : buyQueue)
    // {
    //     while (!pair.second.empty())
    //     {
    //         delete pair.second.top();
    //         pair.second.pop();
    //     }
    // }
    cout << endl;
    cout << "---End of Day--- " << endl;
    cout << "Total Amount of Money Transferred: "
         << "$" << total_transfer << endl;
    cout << "Number of Completed Trades: " << trade_number << endl;
    cout << "Number of Shares Traded: " << total_shares << endl;
    for (auto it = trader_info_map.begin(); it != trader_info_map.end(); ++it)
    {
        auto [key, value] = *it;
        cout << key << " bought " << value.shares_bought << " and sold " << value.shares_sold << " for a net transfer of $" << value.net_transfer << endl;
    }
}

#include <iostream>
#include <fstream>
#include <string>
#include <atomic>
#include <vector>
#include <sstream>
#include <mutex>
#include <thread>
#include "csat.hpp"
#include "CustomHeap.hpp"
#include "CustomHeapS.hpp"
#include "CustomHashMap.hpp"
using namespace std;

std::mutex fileMutex;
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

struct TradeData
{
    int price;
    int quantity;
};
struct CompanyTradeData
{
    std::string company;
    std::vector<TradeData> trades;
};
CustomHashMap<std::string, CompanyTradeData> allTradeData;

CompanyTradeData *findTradeData(const std::string &company)
{
    auto it = allTradeData.find(company);
    if (it != nullptr)
    {
        return &(allTradeData[company]);
    }
    return nullptr; // Company trade data not found
}

void addTrade(const std::string &company, int price, int quantity)
{
    CompanyTradeData *companyTradeData = findTradeData(company);

    if (companyTradeData == nullptr)
    {
        // If the company does not exist in allTradeData, add a new CompanyTradeData
        CompanyTradeData newCompanyTradeData;
        newCompanyTradeData.company = company;
        allTradeData[company] = newCompanyTradeData;
        companyTradeData = &allTradeData[company];
    }

    // Add the new trade data to this company's trades
    TradeData newTrade;
    newTrade.price = price;
    newTrade.quantity = quantity;
    companyTradeData->trades.push_back(newTrade);
}

int partition(vector<TradeData> &trades, int left, int right)
{
    int pivotIndex = left + (right - left) / 2;
    TradeData pivotValue = trades[pivotIndex];
    std::swap(trades[pivotIndex], trades[right]); // Move pivot to end
    int storeIndex = left;

    for (int i = left; i < right; i++)
    {
        if (trades[i].price < pivotValue.price)
        {
            std::swap(trades[i], trades[storeIndex]);
            storeIndex++;
        }
    }
    std::swap(trades[storeIndex], trades[right]); // Move pivot to its final place
    return storeIndex;
}

int quickSelect(vector<TradeData> &trades, int left, int right, int halfQuantity)
{
    if (left == right)
    {
        return trades[left].price;
    }

    int pivotIndex = partition(trades, left, right);
    int totalLeftQuantity = 0;
    for (int i = left; i < pivotIndex; i++)
    {
        totalLeftQuantity += trades[i].quantity;
    }

    int pivotQuantity = trades[pivotIndex].quantity;
    if (halfQuantity >= totalLeftQuantity && halfQuantity < totalLeftQuantity + pivotQuantity)
    {
        return trades[pivotIndex].price;
    }
    else if (halfQuantity < totalLeftQuantity)
    {
        return quickSelect(trades, left, pivotIndex - 1, halfQuantity);
    }
    else
    {
        return quickSelect(trades, pivotIndex + 1, right, halfQuantity - (totalLeftQuantity + pivotQuantity));
    }
}

int calculateWeightedMedianPrice(const std::string &company)
{
    auto tradeDataIt = allTradeData.find(company);
    if (tradeDataIt == nullptr || allTradeData[company].trades.empty())
    {
        return 0; // No trades for this stock
    }

    // Making a copy of trades because quickSelect modifies the vector
    vector<TradeData> tradesCopy = allTradeData[company].trades;
    int totalQuantity = 0;
    for (const auto &trade : tradesCopy)
    {
        totalQuantity += trade.quantity;
    }

    int halfQuantity = totalQuantity / 2;
    return quickSelect(tradesCopy, 0, tradesCopy.size() - 1, halfQuantity);
}
struct info2
{
    int time, price, quantity, expiry;
    std::string trader, action, company;
    int line;
};

struct buy_queue2
{
    CustomHeap heap;
    string company;
};
struct sell_queue2
{
    CustomHeapS heap;
    string company;
};

struct trader_info2
{
    int shares_bought = 0;
    int shares_sold = 0;
};

// Declare the market queues
CustomHashMap<string, CustomPriorityQueue<info *, InfoComparator>> buyQueue;
CustomHashMap<string, CustomPriorityQueue<info *, InfoComparator2>> sellQueue;

CustomHashMap<string, trader_info2 *> trader_info_map2;
int updateMarketState(int &startingLineNumber, string &lastCompany)
{
    std::ifstream inputFile("output.txt");
    std::string line;
    int currentLineNumber = 0;
    while (getline(inputFile, line))
    {
        if (line == "!@")
            return 0;
        currentLineNumber++; // Increment line number with each read line
        if (currentLineNumber <= startingLineNumber)
            continue;
        std::istringstream line_stream(line);
        int time, price, quantity, expiry;
        std::string trader, action;
        std::string component, company;

        // Parse the initial fixed elements
        line_stream >> time >> trader >> action;

        // Split the remaining part of the line into components
        std::vector<std::string> components;
        while (line_stream >> component)
        {
            components.push_back(component);
        }

        // Last three components are price, quantity, expiry
        if (components.size() >= 3)
        {
            price = std::stoi(components[components.size() - 3].substr(1));    // Remove '$'
            quantity = std::stoi(components[components.size() - 2].substr(1)); // Remove '#'
            expiry = std::stoi(components[components.size() - 1]);

            // Everything else is part of the company name
            for (size_t i = 0; i < components.size() - 3; i = ++i)
            {
                if (i > 0)
                    company += " ";
                company += components[i];
                // arbitrageTracker[currentLineNumber].stockName[components[i]] = stoi(components[i + 1]);
            }
            if (components.size() > 4)
            {
                for (size_t i = 0; i < components.size() - 3; i = i = i + 2)
                {
                    if (action == "BUY")
                        arbitrageTracker[currentLineNumber].stockName[components[i]] = stoi(components[i + 1]);
                    else
                        arbitrageTracker[currentLineNumber].stockName[components[i]] = -stoi(components[i + 1]);
                }
            }
            else
            {
                if (action == "BUY")
                    arbitrageTracker[currentLineNumber].stockName[components[0]] = 1;
                else
                    arbitrageTracker[currentLineNumber].stockName[components[0]] = -1;
            }
        }

        info *information = new info;
        information->time = time;
        arbitrageTracker[currentLineNumber].time = time;
        information->price = price;
        information->quantity = quantity;
        information->expiry = (expiry == -1) ? 2477289 : expiry;
        expiry = (expiry == -1) ? 2477289 : expiry;
        information->trader = trader;
        information->action = action;
        information->company = company;
        information->line = currentLineNumber;
        arbitrageTracker[currentLineNumber].price = (action == "SELL") ? price : -price;
        arbitrageTracker[currentLineNumber].quantity = quantity;
        arbitrageTracker[currentLineNumber].expiry = expiry;
        arbitrageTracker[currentLineNumber].trader = trader;
        arbitrageTracker[currentLineNumber].action = action;
        arbitrageTracker[currentLineNumber].company = company;
        if (action == "SELL")
        {
            {
                sellQueue[company].push(information);
            }
        }
        else
        {
            {
                buyQueue[company].push(information);
            }

            // Order matching and expiration check

            while (!sellQueue[company].empty() && !buyQueue[company].empty())
            {

                // cerr << "1" << endl;
                auto seller = sellQueue[company].top();
                auto buyer = buyQueue[company].top();
                // cerr << "2" << endl;
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
                // cerr << "7" << endl;
                if (!seller || !buyer)
                    break;

                if (buyer->price >= seller->price) // Trade occurs
                {
                    // cerr << "5" << endl;
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
                    // cerr << "nimga" << endl;
                    std::cerr << buyer->trader << " purchased " << tradedQuantity << " share of " << company << " from " << seller->trader << "for "
                              << "$" << current_price << "/share" << endl;
                    if (trader_info_map2.find(seller->trader) == nullptr)
                    {
                        trader_info2 *t = new trader_info2;
                        trader_info_map2[seller->trader] = t;
                    }
                    if (trader_info_map2.find(buyer->trader) == nullptr)
                    {
                        trader_info2 *t = new trader_info2;
                        trader_info_map2[buyer->trader] = t;
                    }
                    arbitrageTracker.remove(buyer->line);
                    arbitrageTracker.remove(seller->line);
                    lastCompany = company;
                    addTrade(company, current_price, tradedQuantity);
                    trader_info_map2[buyer->trader]->shares_bought += tradedQuantity;
                    // cerr << "nimga2" << endl;
                    if (buyer->quantity > seller->quantity)
                    {
                        buyer->quantity -= tradedQuantity;
                        sellQueue[company].pop();
                        // delete seller;
                    }
                    else if (buyer->quantity < seller->quantity)
                    {
                        seller->quantity -= tradedQuantity;
                        buyQueue[company].pop();
                        // delete buyer;
                    }
                    else // Equal quantity
                    {
                        sellQueue[company].pop();
                        buyQueue[company].pop();
                        // delete seller;
                        // delete buyer;
                    }
                }
                else
                    break; // Prices don't match, so no trade occurs
            }
        }
        if (currentLineNumber == startingLineNumber)
            return 1;
        startingLineNumber = currentLineNumber;
        return 2;
    }
    return 1;
}
void makeTradingDecision(int currentTime, int &startingLineNumber, string lastCompany)
{
    int time;
    int medianPrice = calculateWeightedMedianPrice(lastCompany);
    if (!sellQueue[lastCompany].empty())
    {
        auto seller = sellQueue[lastCompany].top();
        // Accept sell orders below median price
        if (seller->price < medianPrice)
        {

            // cerr << medianPrice << endl;
            // Process this sell order
            time = seller->expiry + seller->time - currentTime;

            if (seller->expiry > 247729)
            {
                time = -1;
            }
            else

                std::cout << currentTime << " 22b1060_22b1052 "
                          << "BUY " << lastCompany
                          << " $" << seller->price << " #" << seller->quantity << " " << time << endl;
            // std::cerr << currentTime << " 22b1060_22b1052 "
            //           << "BUY " << lastCompany
            //           << " $" << seller->price << " #" << seller->quantity << " " << seller->expiry + seller->time - currentTime << endl;
            addTrade(lastCompany, seller->price, seller->quantity);
            startingLineNumber++;
            sellQueue[lastCompany].pop();
            // Update your trader info, process the trade, etc.
            //...
        }
    }
    medianPrice = calculateWeightedMedianPrice(lastCompany);
    if (!buyQueue[lastCompany].empty())
    {
        auto buyer = buyQueue[lastCompany].top();
        // Buy if there is a buy order lower than the median price
        if (buyer->price > medianPrice)
        {
            time = buyer->expiry + buyer->time - currentTime;

            if (buyer->expiry == 2477289)
            {
                time = -1;
            }
            addTrade(lastCompany, buyer->price, buyer->quantity);
            startingLineNumber++;
            std::cout << currentTime << " 22b1060_22b1052 "
                      << "SELL " << lastCompany
                      << " $" << buyer->price << " #" << buyer->quantity << " " << time << endl;
            // std::cerr << currentTime << " 22b1060_22b1052 "
            //           << "SELL " << lastCompany
            //           << " $" << buyer->price << " #" << buyer->quantity << " " << buyer->expiry + buyer->time - currentTime << endl;
            buyQueue[lastCompany].pop();
        }
    }
}

void processArbitrageTracker(CustomHashMap<int, LinearCombination> &arbitrageTracker, int currentTime, int &currentLine)
{
    ArbitrageSolver solver;

    for (size_t i = 0; i < arbitrageTracker.bucketCount(); ++i)
    {
        CustomHashMap<int, LinearCombination>::HashNode *node = arbitrageTracker.bucketHead(i);
        while (node != nullptr)
        {
            const LinearCombination &lc = node->value; // Access the value
            solver.addLinearCombinationTrade(lc, currentLine);

            node = node->next; // Move to the next node in the current bucket
        }
    }

    if (solver.findArbitrage())
    {
        solver.printSuccessfulArbitrage(currentTime, currentLine);
    }
    else
    {
        std::cerr << "No arbitrage opportunity found." << std::endl;
    }
}

int reader(int time, int &startingLineNumber, string &lastCompany)
{
    int a = updateMarketState(startingLineNumber, lastCompany);
    if (!a)
        return 0;
    makeTradingDecision(time, startingLineNumber, lastCompany);
    if (a == 2)
    {
        // Run in a separate thread
        std::thread arbThread(processArbitrageTracker, std::ref(arbitrageTracker), time, std::ref(startingLineNumber));
        arbThread.detach();
    }
    return 1; // Continue running
}
// void arbitrage(){

// }
void *userThread(void *arg)
{
    string lastCompany = "";
    int startingLineNumber = 0;
    int thread_id = *(int *)arg;
    while (true)
    {
        int currentTime = commonTimer.load();
        if (!reader(currentTime, startingLineNumber, lastCompany))
            break;
    }
    return nullptr;
}
// bool linearCombination()
// {
//     sleep(1);
//     std::cout << "hi" << endl;
//     return true;
// }

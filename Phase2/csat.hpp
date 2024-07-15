#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "atomic"
#include "CustomHashMap.hpp"
extern std::atomic<int> commonTimer;
using namespace std;
struct LinearCombination
{
    CustomHashMap<string, int> stockName;
    int time, price, quantity, expiry;
    std::string trader, action, company;
};
CustomHashMap<int, LinearCombination> arbitrageTracker;
class ArbitrageSolver
{
private:
    struct Trade
    {
        std::vector<int> quantities;
        int profit;
        int maxMultiplier;
        std::string rawTrade;
        bool action; // true for sell, false for buy
        int lineNumber;
    };

    std::vector<Trade> trades;
    std::vector<std::string> stockNames; // Store stock names
    std::vector<int> stockIndices;       // Store corresponding indices
    std::vector<int> successfulMultipliers;
    int getIndexForStock(const std::string &stock)
    {
        for (size_t i = 0; i < stockNames.size(); i++)
        {
            if (stockNames[i] == stock)
                return i;
        }
        stockNames.push_back(stock);
        stockIndices.push_back(stockNames.size() - 1);
        return stockIndices.back();
    }

    bool isProfitableAndBalanced(const std::vector<int> &multipliers)
    {
        std::vector<int> netStocks(trades[0].quantities.size(), 0);
        int netProfit = 0;

        for (size_t i = 0; i < trades.size(); ++i)
        {
            int multiplier = multipliers[i];
            netProfit += trades[i].profit * multiplier;
            for (size_t j = 0; j < trades[i].quantities.size(); ++j)
            {
                netStocks[j] += trades[i].quantities[j] * multiplier;
            }
        }

        if (netProfit <= 0)
            return false;
        for (int qty : netStocks)
            if (qty != 0)
                return false;
        return true;
    }

    bool backtrack(std::vector<int> &multipliers, size_t tradeIndex, std::vector<int> &highestMultipliers)
    {
        if (tradeIndex == trades.size())
        {
            if (isProfitableAndBalanced(multipliers))
            {
                // Update to higher multipliers if found
                highestMultipliers = multipliers;
                return true;
            }
            return false;
        }

        bool found = false;
        int maxProfit = 0;
        size_t maxProfitIndex = 0;

        // Heuristic: Find the trade with the highest potential profit
        for (size_t i = tradeIndex; i < trades.size(); ++i)
        {
            int potentialProfit = trades[i].profit * trades[i].maxMultiplier;
            if (potentialProfit > maxProfit)
            {
                maxProfit = potentialProfit;
                maxProfitIndex = i;
            }
        }

        // Swap the trade with the highest potential profit to the current index
        std::swap(trades[tradeIndex], trades[maxProfitIndex]);
        std::swap(multipliers[tradeIndex], multipliers[maxProfitIndex]);

        int maxMultiplier = trades[tradeIndex].maxMultiplier;
        for (int multiplier = 0; multiplier <= maxMultiplier; ++multiplier)
        {
            multipliers[tradeIndex] = multiplier;
            found |= backtrack(multipliers, tradeIndex + 1, highestMultipliers);
        }

        // Swap back after backtracking
        std::swap(trades[tradeIndex], trades[maxProfitIndex]);
        std::swap(multipliers[tradeIndex], multipliers[maxProfitIndex]);

        return found;
    }
    void resetTradeBook()
    {
        // Reset the trade book when it reaches a certain size or condition
        if (trades.size() > 5)
        {
            trades.clear();
            stockNames.clear();
            stockIndices.clear();
            arbitrageTracker.clear();
        }
    }

public:
    ArbitrageSolver() {}

    void addTrade(const std::string &tradeStr, int maxMultiplier)
    {
        resetTradeBook();
        if (trades.size() > 10)
        {
            trades.clear();
            arbitrageTracker.clear();
        }
        std::istringstream iss(tradeStr);
        std::string token;
        std::vector<std::pair<std::string, int>> tradeStocks;
        int profit;

        // Skip trader name and action (BUY/SELL)
        iss >> token >> token;

        // Parse the stock quantities
        while (iss >> token && token[0] != '$')
        {
            int quantity;
            iss >> quantity;
            tradeStocks.push_back({token, quantity});
        }

        // Extract and parse the price (remove '$' before conversion)
        std::string priceStr = token.substr(1);
        profit = std::stoi(priceStr);
        iss >> token; // Skip quantity

        Trade trade;
        trade.rawTrade = tradeStr;
        trade.profit = profit;
        trade.maxMultiplier = maxMultiplier;
        trade.quantities.resize(stockNames.size(), 0); // Resize based on the number of different stocks

        for (const auto &[stock, quantity] : tradeStocks)
        {
            int index = getIndexForStock(stock);
            if (index >= trade.quantities.size())
            {
                trade.quantities.resize(index + 1, 0); // Ensure sufficient size
            }
            trade.quantities[index] = quantity;
        }

        // Resize quantities for all existing trades if a new stock was added
        for (auto &t : trades)
        {
            t.quantities.resize(stockNames.size(), 0);
        }

        trades.push_back(trade);
    }

    bool findArbitrage()
    {
        if (trades.empty())
            return false;

        std::vector<int> multipliers(trades.size(), 0);
        std::vector<int> highestMultipliers(trades.size(), 0); // Initialize with lowest multipliers
        bool found = backtrack(multipliers, 0, highestMultipliers);

        if (found)
        {
            successfulMultipliers = highestMultipliers; // Update successful multipliers to the highest found
        }
        return found;
    }

    void printSuccessfulArbitrage(int currentTime, int &currentLine)
    {
        if (successfulMultipliers.empty())
        {
            // std::cout << "No arbitrage opportunity found." << std::endl;
            return;
        }
        std::cerr << "Arbitrage opportunity found:" << std::endl;
        for (size_t i = trades.size(); i > 0; --i)
        {
            if (successfulMultipliers[i] > 0)
            {
                int profit = trades[i].profit;
                int Quantity = successfulMultipliers[i];
                // arbitrageTracker[trades[i].lineNumber].quantity -= Quantity;

                // if (arbitrageTracker[trades[i].lineNumber].quantity == 0)
                //     arbitrageTracker.erase(trades[i].lineNumber);
                if (trades[i].action)
                {
                    profit = -profit;
                    for (auto i : trades[i].quantities)
                    {
                        i = -i;
                    }
                }
                int currentTime = commonTimer.load();
                int t = arbitrageTracker[trades[i].lineNumber].time + arbitrageTracker[trades[i].lineNumber].expiry - currentTime;
                if (arbitrageTracker[trades[i].lineNumber].expiry == -1)
                    t = -1;
                cerr << arbitrageTracker[trades[i].lineNumber].expiry << endl;
                // if (t != -1 && t < currentTime)
                //     break;

                cout << currentTime << " " << trades[i].rawTrade << " $" << profit << " #" << Quantity << " " << t << endl;
                std::cerr << "Trade: " << trades[i].rawTrade << " | Multiplier: " << successfulMultipliers[i] << std::endl;
                trades[i].maxMultiplier = 0;
                arbitrageTracker.remove(trades[i].lineNumber);
            }
        }
        trades.clear();
    }
    void addLinearCombinationTrade(const LinearCombination &lc, int line)
    {
        Trade trade;
        trade.lineNumber = line;
        std::string action = (lc.action == "SELL") ? "BUY" : "SELL";
        trade.rawTrade = "Kunal " + action + " " + lc.company;
        trade.profit = lc.price;
        trade.maxMultiplier = lc.quantity;
        trade.action = (lc.action == "SELL") ? false : true;

        // Resize the quantities vector for the new trade
        trade.quantities.resize(stockIndices.size(), 0); // Assuming getKeys returns a list of keys
        for (size_t bucket = 0; bucket < lc.stockName.bucketCount(); ++bucket)
        {
            for (auto node = lc.stockName.bucketHead(bucket); node != nullptr; node = node->next)
            {
                const auto &stockName = node->key;
                int quantity = node->value;

                int index = getIndexForStock(stockName);
                if (index >= trade.quantities.size())
                {
                    trade.quantities.resize(index + 1, 0);
                }

                trade.quantities[index] = quantity;
            }
        }
        // Resize quantities for all existing trades if a new stock was added
        for (auto &t : trades)
        {
            t.quantities.resize(stockNames.size(), 0);
        }

        // Add the new trade to the list of trades
        trades.push_back(trade);
    }
};

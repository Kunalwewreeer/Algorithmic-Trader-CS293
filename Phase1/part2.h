#ifndef PART2_H
#define PART2_h
using namespace std;
#include <string>
#include <vector>
#include <sstream>
struct spair2
{
    string stockName;
    int quantity;
};

struct orderInfo2
{
    vector<spair2> stock;
    int netQuantity = 0;
    int netPrice = 0;
    bool buyOrSell; // 0 for buy and 1 for sell
    int LineNumber = -1;
};
std::ostream &operator<<(std::ostream &os, const orderInfo2 &order)
{
    for (const auto &[stockName, quantity] : order.stock)
    {
        os << stockName << " " << quantity << " ";
    }
    os << order.netPrice << " " << order.netQuantity << " ";
    os << (order.buyOrSell ? "s#" : "b#");
    return os;
}

bool areOppositeOrders2(const orderInfo2 &order1, const orderInfo2 &order2)
{
    if (order1.buyOrSell == order2.buyOrSell || order1.netPrice != -order2.netPrice)
    {
        return false;
    }

    // Ensure the sizes of the stock lists are equal
    if (order1.stock.size() != order2.stock.size())
    {
        return false;
    }

    // Check each stock in order1 against order2
    for (const auto &stock1 : order1.stock)
    {
        bool matchFound = false;
        for (const auto &stock2 : order2.stock)
        {
            if (stock1.stockName == stock2.stockName && stock1.quantity == -stock2.quantity)
            {
                matchFound = true;
                break;
            }
        }
        if (!matchFound)
        {
            return false;
        }
    }
    return true;
}
bool stocksEqual2(const vector<spair2> &stock1, const vector<spair2> &stock2)
{
    if (stock1.size() != stock2.size())
    {
        return false;
    }

    for (size_t i = 0; i < stock1.size(); ++i)
    {
        if (stock1[i].stockName != stock2[i].stockName || stock1[i].quantity != stock2[i].quantity)
        {
            return false;
        }
    }

    return true;
}

int processOrder2(vector<orderInfo2> &stockBook, const orderInfo2 &order)
{
    if (stockBook.empty())
        return -1; // Safety check if stockBook is empty

    // Iterate up to the second-to-last element
    for (size_t i = 0; i < stockBook.size() - 1; ++i)
    {
        auto &item = stockBook[i];

        if (stocksEqual2(item.stock, order.stock) && item.netQuantity == order.netQuantity && item.buyOrSell == order.buyOrSell)
        {
            if (item.netPrice > order.netPrice)
            {
                // Reset the order
                item.stock.clear();
                item.netQuantity = 0;
                item.netPrice = 0;
                item.buyOrSell = false; // Default value
                return 0;               // Indicate that the order has been erased
            }
            else if (item.netPrice > order.netPrice)
            {
                return -2; // Indicate lower netPrice
            }
        }
    }
    return -1; // No matching order found
}
// Returns the LineNumber of the matching order, or -1 if no match is found
int findAndUpdateMatchingOrder2(orderInfo2 &currentOrder, std::vector<orderInfo2> &stockBook)
{
    for (size_t i = 0; i < stockBook.size(); ++i)
    {
        orderInfo2 &order = stockBook[i];

        if (i != currentOrder.LineNumber && areOppositeOrders2(currentOrder, order))
        {
            if (order.netQuantity <= currentOrder.netQuantity)
            {
                currentOrder.netQuantity -= order.netQuantity;
                order = orderInfo2(); // Reset the order to default if it's fully matched

                if (currentOrder.netQuantity == 0)
                {
                    stockBook[currentOrder.LineNumber] = orderInfo2(); // Reset current order if fully matched
                    return -2;                                         // Return the lineNumber of the matched order.
                }
            }
            else
            {
                order.netQuantity -= currentOrder.netQuantity;
                currentOrder = orderInfo2(); // Reset current order if fully matched
                return -2;                   // Return the lineNumber of the matched order.
            }
        }
    }
    return -1; // Indicating no match found.
}

// Structure to store successful combinations
struct combinationResult2
{
    vector<int> usedLines;
    int netPriceSum;

    combinationResult2(int size = 100) : netPriceSum(0)
    {
        usedLines.resize(size, 0);
    }
};

bool allStocksZero2(const std::vector<spair2> &stock)
{
    for (const auto &pair : stock)
    {
        if (pair.quantity != 0)
            return false;
    }
    return true;
}
hashmap calculateMaxQuantities2(const std::vector<orderInfo2> &stockBook, int maxLineNumber);
void findCombinations2(int lineNumber, const std::vector<orderInfo2> &stockBook,
                       orderInfo2 current, std::vector<int> currentLines,
                       std::vector<combinationResult2> &results,
                       const hashmap &remainingQuantities)
{
    if (lineNumber < 0)
    {
        if (allStocksZero2(current.stock) && current.netPrice >= 0)
        {
            combinationResult2 result(stockBook.size());
            for (size_t i = 0; i < currentLines.size(); ++i)
            {
                result.usedLines[i] = currentLines[i];
            }
            result.netPriceSum = current.netPrice;
            results.push_back(result);
        }
        return;
    }

    if (static_cast<size_t>(lineNumber) >= stockBook.size())
    {
        return; // Skip if the lineNumber is out of bounds
    }

    // Recursive call without including the current line
    // findCombinations2(lineNumber - 1, stockBook, current, currentLines, results, remainingQuantities);

    const auto &order = stockBook[lineNumber];

    for (int i = 0; i <= order.netQuantity; ++i)
    {

        orderInfo2 newCombination = current;
        std::vector<int> newLines = currentLines; // Copy currentLines
        hashmap newRemainingQuantities = remainingQuantities;
        bool feasible = true;

        for (const auto &pair : order.stock)
        {
            int adjustedQuantity = pair.quantity * i;
            bool found = false;

            // Update or add the stock in the new combination
            for (auto &sp : newCombination.stock)
            {
                if (sp.stockName == pair.stockName)
                {
                    sp.quantity += adjustedQuantity;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                newCombination.stock.push_back({pair.stockName, adjustedQuantity});
            }

            newRemainingQuantities[pair.stockName] -= std::abs(adjustedQuantity);
        }

        // Check feasibility after processing all stocks
        for (const auto &sp : newCombination.stock)
        {
            if (std::abs(sp.quantity) > std::abs(newRemainingQuantities[sp.stockName]))
            {
                feasible = false;
                break;
            }
        }

        if (!feasible)
            continue;

        newCombination.netPrice += order.netPrice * i;
        newLines[lineNumber] = i; // Update used line quantity

        // Recursive call including the current line
        findCombinations2(lineNumber - 1, stockBook, newCombination, newLines, results, newRemainingQuantities);
    }
}

// Function to calculate maximum possible quantities for each stock
hashmap calculateMaxQuantities2(const std::vector<orderInfo2> &stockBook, int maxLineNumber)
{
    hashmap maxQuantities;
    for (const auto &order : stockBook)
    {
        if (order.LineNumber > maxLineNumber)
            return maxQuantities;
        for (const auto &pair : order.stock)
        {
            maxQuantities[pair.stockName] += std::abs(pair.quantity) * order.netQuantity;
        }
    }
    return maxQuantities;
}

// Modify the findBestCombination2 function to use the new findCombinations2 logic
combinationResult2 findBestCombination2(const std::vector<orderInfo2> &stockBook, int maxLineNumber)
{
    std::vector<combinationResult2> results;
    orderInfo2 initial;
    std::vector<int> initialLines(maxLineNumber + 1, 0); // Initialize with zeroes

    auto maxQuantities = calculateMaxQuantities2(stockBook, maxLineNumber);

    findCombinations2(maxLineNumber, stockBook, initial, initialLines, results, maxQuantities);

    combinationResult2 bestCombination(maxLineNumber);
    int maxNetPriceSum = 0;

    for (const auto &result : results)
    {
        if (result.netPriceSum < maxNetPriceSum)
        {
            bestCombination = result;
            maxNetPriceSum = result.netPriceSum;
        }
    }
    return bestCombination;
}

void updateStockBook2(std::vector<orderInfo2> &stockBook, const combinationResult2 &bestCombination, int maxLineNumber)
{
    for (int i = 0; i <= maxLineNumber; ++i)
    {
        int usedQuantity = bestCombination.usedLines[i];
        if (usedQuantity > 0)
        {
            orderInfo2 &order = stockBook[i];
            order.netQuantity -= usedQuantity;
            if (order.netQuantity <= 0)
            {
                order.stock.clear();
                order.netQuantity = 0;
                order.netPrice = 0;
                order.buyOrSell = false;
                order.LineNumber = -1;
            }
        }
    }
}

bool processInput2(std::vector<orderInfo2> &stockBook, int lineNumber, const std::string &inputLine, int &netProfit)
{
    orderInfo2 newOrder;
    std::stringstream ss(inputLine);
    std::string stockName, buyOrSell;
    int quantity, netPrice, netQuantity;
    std::vector<std::string> tokens;

    // Parse inputLine for tokens
    while (ss >> stockName)
    {
        tokens.push_back(stockName);
    }

    // Extract netQuantity, netPrice, and buyOrSell from the last three elements
    buyOrSell = tokens.back();
    netPrice = std::stoi(tokens[tokens.size() - 2]);

    // Process stock names and quantities
    for (size_t i = 0; i < tokens.size() - 2; i += 2)
    {
        quantity = std::stoi(tokens[i + 1]);
        newOrder.stock.push_back({tokens[i], quantity});
    }
    newOrder.netQuantity = 1;
    newOrder.LineNumber = lineNumber;

    // Adjust quantities based on buy or sell
    newOrder.buyOrSell = (buyOrSell == "b#");
    if (newOrder.buyOrSell)
    {
        for (auto &sq : newOrder.stock)
        {
            sq.quantity = -sq.quantity;
        }
        netPrice = -netPrice;
    }
    newOrder.netPrice = netPrice;

    // Ensure stockBook has enough space
    if (lineNumber >= stockBook.size())
    {
        stockBook.resize(lineNumber + 1);
    }

    // Insert the new order into the stockBook
    stockBook[lineNumber] = newOrder;

    // Check for and update matching orders
    int matchingLineNumber = findAndUpdateMatchingOrder2(newOrder, stockBook);
    if (matchingLineNumber == -2)
    {
        return false;
    }
    int flag = processOrder2(stockBook, newOrder);
    if (flag == -2)
        return false;
    // Find the best combination for arbitrage
    combinationResult2 bestCombination = findBestCombination2(stockBook, lineNumber);
    if (bestCombination.netPriceSum <= 0)
        return false;
    netProfit += bestCombination.netPriceSum;
    for (int i = lineNumber; i >= 0; --i)
    {
        if (bestCombination.usedLines[i] == 0)
            continue;

        const orderInfo2 &info = stockBook[i];

        for (const auto &stock : info.stock)
        {
            int adjustedQuantity = stock.quantity;
            if (info.buyOrSell)
                adjustedQuantity = -adjustedQuantity;
            cout << stock.stockName << " " << adjustedQuantity << " ";
        }

        cout << info.netPrice << " ";
        cout << (info.buyOrSell ? "b#" : "s#") << endl;
    }

    // Update the stock book with the best combination found
    updateStockBook2(stockBook, bestCombination, lineNumber);
    // cout << netProfit << endl;
    return true;
} // auto start = std::chrono::high_resolution_clock::now();
//---------------------------------
#endif

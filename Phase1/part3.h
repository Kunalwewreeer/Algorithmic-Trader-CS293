#ifndef PART3_H
#define PART3_H
using namespace std;
#include <string>
#include <vector>
#include <sstream>

struct spair
{
    string stockName;
    int quantity;
};
struct orderInfo
{
    vector<spair> stock;
    int netQuantity = 0;
    int netPrice = 0;
    bool buyOrSell; // 0 for buy and 1 for sell
    int LineNumber;
};
std::ostream &operator<<(std::ostream &os, const orderInfo &order)
{
    for (const auto &[stockName, quantity] : order.stock)
    {
        os << stockName << " " << quantity << " ";
    }
    os << order.netPrice << " " << order.netQuantity << " ";
    os << (order.buyOrSell ? "s#" : "b#");
    return os;
}

bool areOppositeOrders(const orderInfo &order1, const orderInfo &order2)
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

// Returns the LineNumber of the matching order, or -1 if no match is found
int findAndUpdateMatchingOrder(orderInfo &currentOrder, std::vector<orderInfo> &stockBook)
{
    for (size_t i = 0; i < stockBook.size(); ++i)
    {
        orderInfo &order = stockBook[i];

        if (i != currentOrder.LineNumber && areOppositeOrders(currentOrder, order))
        {
            if (order.netQuantity <= currentOrder.netQuantity)
            {
                currentOrder.netQuantity -= order.netQuantity;
                order = orderInfo(); // Reset the order to default if it's fully matched

                if (currentOrder.netQuantity == 0)
                {
                    stockBook[currentOrder.LineNumber] = orderInfo(); // Reset current order if fully matched
                    return -2;                                        // Return the lineNumber of the matched order.
                }
            }
            else
            {
                order.netQuantity -= currentOrder.netQuantity;
                currentOrder = orderInfo(); // Reset current order if fully matched
                return -2;                  // Return the lineNumber of the matched order.
            }
        }
    }
    return -1; // Indicating no match found.
}
// Structure to store successful combinations
struct combinationResult
{
    vector<int> usedLines;
    int netPriceSum;

    combinationResult(int size = 100) : netPriceSum(0)
    {
        usedLines.resize(size, 0);
    }
};

bool allStocksZero(const std::vector<spair> &stock)
{
    for (const auto &pair : stock)
    {
        if (pair.quantity != 0)
            return false;
    }
    return true;
}
hashmap calculateMaxQuantities(const std::vector<orderInfo> &stockBook, int maxLineNumber);
void findCombinations(int lineNumber, const std::vector<orderInfo> &stockBook,
                      orderInfo current, std::vector<int> currentLines,
                      std::vector<combinationResult> &results,
                      const hashmap &remainingQuantities)
{
    if (lineNumber < 0)
    {
        if (allStocksZero(current.stock) && current.netPrice > 0)
        {
            combinationResult result(stockBook.size());
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
    findCombinations(lineNumber - 1, stockBook, current, currentLines, results, remainingQuantities);

    const auto &order = stockBook[lineNumber];

    for (int i = 1; i <= order.netQuantity; ++i)
    {

        orderInfo newCombination = current;
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
        findCombinations(lineNumber - 1, stockBook, newCombination, newLines, results, newRemainingQuantities);
    }
}

// Function to calculate maximum possible quantities for each stock
hashmap calculateMaxQuantities(const std::vector<orderInfo> &stockBook, int maxLineNumber)
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

// Modify the findBestCombination function to use the new findCombinations logic
combinationResult findBestCombination(const std::vector<orderInfo> &stockBook, int maxLineNumber)
{
    std::vector<combinationResult> results;
    orderInfo initial;
    std::vector<int> initialLines(maxLineNumber + 1, 0); // Initialize with zeroes

    auto maxQuantities = calculateMaxQuantities(stockBook, maxLineNumber);

    findCombinations(maxLineNumber, stockBook, initial, initialLines, results, maxQuantities);

    combinationResult bestCombination(maxLineNumber);
    int maxNetPriceSum = 0;
    for (const auto &result : results)
    {
        if (result.netPriceSum > maxNetPriceSum)
        {
            bestCombination = result;
            maxNetPriceSum = result.netPriceSum;
        }
    }
    return bestCombination;
}

void updateStockBook(std::vector<orderInfo> &stockBook, const combinationResult &bestCombination, int maxLineNumber)
{
    for (int i = 0; i <= maxLineNumber; ++i)
    {
        int usedQuantity = bestCombination.usedLines[i];
        if (usedQuantity > 0)
        {
            orderInfo &order = stockBook[i];
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

bool processInput(std::vector<orderInfo> &stockBook, int lineNumber, const std::string &inputLine, int &netProfit)
{
    orderInfo newOrder;
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
    netQuantity = std::stoi(tokens[tokens.size() - 2]);
    netPrice = std::stoi(tokens[tokens.size() - 3]);

    // Process stock names and quantities
    for (size_t i = 0; i < tokens.size() - 3; i += 2)
    {
        quantity = std::stoi(tokens[i + 1]);
        newOrder.stock.push_back({tokens[i], quantity});
    }
    newOrder.netQuantity = netQuantity;
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
    int matchingLineNumber = findAndUpdateMatchingOrder(newOrder, stockBook);
    if (matchingLineNumber == -2)
    {
        return false;
    }

    // Find the best combination for arbitrage
    combinationResult bestCombination = findBestCombination(stockBook, lineNumber);
    if (bestCombination.netPriceSum <= 0)
        return false;
    netProfit += bestCombination.netPriceSum;

    for (int i = lineNumber; i >= 0; --i)
    {
        if (bestCombination.usedLines[i] == 0)
            continue;

        const orderInfo &info = stockBook[i];

        for (const auto &stock : info.stock)
        {
            int adjustedQuantity = stock.quantity;
            if (info.buyOrSell)
                adjustedQuantity = -adjustedQuantity;
            cout << stock.stockName << " " << adjustedQuantity << " ";
        }

        cout << info.netPrice << " " << bestCombination.usedLines[i] << " ";
        cout << (info.buyOrSell ? "b#" : "s#") << endl;
    }

    // Update the stock book with the best combination found
    updateStockBook(stockBook, bestCombination, lineNumber);
    // cout << netProfit << endl;
    return true;
} // auto start = std::chrono::high_resolution_clock::now();
//---------------------------------

#endif
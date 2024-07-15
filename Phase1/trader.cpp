#include "hashmap.hpp"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sstream>
#include <unistd.h>
#include "receiver.h"
#include "part1.h"
#include "part2.h"
#include "part3.h"
using namespace std;

int main(int argc, char *argv[])
{
    Receiver rcv;
    string runcode = "";
    runcode += argv[1][0];
    if (runcode.compare("1") == 0)
    {

        // sleep(5);

        std::istringstream iss;
        std::string line;
        StockTrie trie;
        std::string residual = "";

        // Combine the leftover data from the previous iteration with the new data.

        while (true)
        {
            string message = rcv.readIML();
            if (message.empty())
            {
                sleep(1); // Doing this so that if the message hasnt been sent yet, so that we can wait till it arrives
            }
            message = residual + message; // To concatenate the last message residual string to updated string recieved
            iss.clear();
            iss.str(message);
            while (getline(iss, line, '#'))
            {

                if (line == "$")
                {
                    goto end_process; // Need to break out of all the loops
                }
                istringstream line_stream(line); // line_stream will break out lines in 3 parts using space as a delimiter
                string stockName, action;
                int price;
                line_stream >> stockName >> price >> action; // Process components: stockName, price, and action
                info *result = trie.search(stockName);       // searching in the trie
                if (result)                                  // If stockname already in the trie, need to update their
                {
                    if (action == "s") // logic not commented
                    {
                        if (price == result->cancelBuy_p)
                        {
                            if (price > result->sell_p)
                            {
                                cout << "No Trade" << endl;
                                continue;
                            }
                            // result->trade_p = price;
                            result->cancelBuy_p = -2147483647;
                            result->buy_p = price;
                            if (price < result->sell_p)
                            {
                                result->cancelSell_p = +2147483647;
                                result->sell_p = price;
                            }
                            cout << "No Trade" << endl;

                            continue;
                        }
                        else if (price < result->trade_p)
                        {
                            if (price >= result->cancelSell_p)
                            {
                                cout << "No Trade" << endl;
                                continue;
                            }
                            result->trade_p = price;
                            result->sell_p = price;
                            if (result->sell_p < result->cancelSell_p)
                            {
                                result->cancelSell_p = +2147483647;
                            }
                            cout << stockName << " " << price << " b" << endl;
                        }

                        else
                        {
                            if (result->sell_p >= price)
                            {
                                result->sell_p = price;
                                result->cancelSell_p = price;
                            }
                            cout << "No Trade" << endl;
                        }
                    }

                    if (action == "b")
                    {
                        if (price == result->cancelSell_p)
                        {
                            if (price < result->buy_p)
                            {
                                cout << "No Trade" << endl;
                                continue;
                            }
                            // result->trade_p = price;
                            result->cancelSell_p = +2147483647;
                            result->sell_p = price;
                            if (price > result->buy_p)
                            {
                                result->cancelBuy_p = -2147483647;
                                result->buy_p = price;
                            }
                            cout << "No Trade" << endl;

                            continue;
                        }
                        else if (price > result->trade_p)
                        {
                            if (price <= result->cancelBuy_p)
                            {
                                cout << "No Trade" << endl;
                                continue;
                            }
                            result->trade_p = price;
                            result->buy_p = price;
                            if (result->buy_p > result->cancelBuy_p)
                            {

                                result->cancelBuy_p = -2147483647;
                            }

                            cout << stockName << " " << price << " s" << endl;
                        }

                        else
                        {
                            if (result->buy_p <= price)
                            {
                                result->buy_p = price;
                                result->cancelBuy_p = price;
                            }
                            cout << "No Trade" << endl;
                        }
                    }
                }

                else // If stockName not in Trie, insert it in the Trie
                {
                    info *new_stock = new info;
                    new_stock->name = stockName;
                    new_stock->trade_p = price;

                    if (action == "s")
                    {
                        action = "b";
                    }
                    else
                    {
                        action = "s";
                    }

                    trie.insert(new_stock);
                    cout << new_stock->name << " " << price << " " << action << endl;
                }
            }
            residual = line;
        }
    end_process:
        return 0;
    }
    if (runcode.compare("2") == 0)
    {
        // auto start = std::chrono::high_resolution_clock::now();
        int netProfit = 0;
        vector<orderInfo2> stockBook;

        // sleep(5);

        std::istringstream iss;
        std::string line;
        std::string residual = "";
        int lineNumber = 0;

        while (true)
        {
            string message = rcv.readIML();
            if (message.empty())
            {
                sleep(1); // Doing this so that if the message hasnt been sent yet, so that we can wait till it arrives
            }
            message = residual + message; // To concatenate the last message residual string to updated string recieved
            iss.clear();
            iss.str(message);
            while (getline(iss, line, '#'))
            {
		istringstream iss1(line);
		char first;
		iss1 >> first;
                if (first == '$' )
                {
                    goto end_process2; // Need to break out of all the loops
                }
                char &lastChar = line.back();
                if (lastChar == 'b')
                {
                    lastChar = 's';
                }
                else if (lastChar == 's')
                {
                    lastChar = 'b';
                }
                line += "#";
                if (line == "$")
                {
                    break;
                }

                if (processInput2(stockBook, lineNumber, line, netProfit) == false)
                {
                    cout << "No Trade" << endl;
                };
                lineNumber++;
                // auto end = std::chrono::high_resolution_clock::now();

                // // Calculating the duration and converting it to milliseconds
                // std::chrono::duration<double, std::milli> duration = end - start;

                // cout << "Time taken: " << duration.count() << " ms" << endl;
            }
            residual = line;
        }
    end_process2:
        cout << netProfit;
        return 0;
    }
    if (runcode.compare("3") == 0)
    {
        // auto start = std::chrono::high_resolution_clock::now();
        int netProfit = 0;
        vector<orderInfo> stockBook;

        std::istringstream iss;
        std::string line;
        std::string residual = "";
        int lineNumber = 0;

        while (true)
        {
            string message = rcv.readIML();
            if (message.empty())
            {
                sleep(1); // Doing this so that if the message hasnt been sent yet, so that we can wait till it arrives
            }
            message = residual + message; // To concatenate the last message residual string to updated string recieved
            iss.clear();
            iss.str(message);
            while (getline(iss, line, '#'))
            {

		istringstream iss1(line);
		char first;
		iss1 >> first;
                if (first == '$' )
                {
                    goto end_process3; // Need to break out of all the loops
                }
                char &lastChar = line.back();
                if (lastChar == 'b')
                {
                    lastChar = 's';
                }
                else if (lastChar == 's')
                {
                    lastChar = 'b';
                }
                line += "#";
                if (line == "$")
                {
                    break;
                }

                if (processInput(stockBook, lineNumber, line, netProfit) == false)
                {
                    cout << "No Trade" << endl;
                };
                lineNumber++;
                // auto end = std::chrono::high_resolution_clock::now();

                // // Calculating the duration and converting it to milliseconds
                // std::chrono::duration<double, std::milli> duration = end - start;

                // cout << "Time taken: " << duration.count() << " ms" << endl;
            }
            residual = line;
        }
    end_process3:
        cout << netProfit;
        return 0;
    }
}

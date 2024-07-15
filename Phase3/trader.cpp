// Listening to a given port no 8888 and printing the incoming messages
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fstream>
#include <string>
#include <arpa/inet.h>
#include <pthread.h>
#include <vector>
#include <sstream>
#include <sys/stat.h>
#include <mutex>
#include <atomic>
#include "CustomHashMap.hpp"
using namespace std;

extern std::atomic<int> commonTimer;
mutex fileMutex;
const int BUFFER_SIZE = 1024;
int NUM_THREADS = 2;
string myclient_name = "kunalc_mayankmotwani";
// struct buy_queue
// {
//     CustomHeap *heap;
//     string company;
// };
struct info
{
    int time, price, quantity, expiry, threadNumber;
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

// struct sell_queue
// {
//     CustomHeapS *heap;
//     string company;
// };

int time1 = 0;
int lineNo = 0;
// Structure to store client socket and its details
struct ClientInfo
{
    int socket;
    struct sockaddr_in address;
    int threadNumber;
    ClientInfo(int socket, struct sockaddr_in &address, int thread) : socket(socket), address(address), threadNumber(thread) {}
    ClientInfo(){};
};
void CookWriteOrder()

{
}

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
static int netProfit = 0;
struct trader_info
{
    int shares_bought = 0;
    int shares_sold = 0;
    int net_transfer = 0;
};

CustomHashMap<string, CustomPriorityQueue<info *, InfoComparator>> buyQueue;
CustomHashMap<string, CustomPriorityQueue<info *, InfoComparator2>> sellQueue;
void updateTraceFile(const char *message, int threadNumber)
{
    std::ofstream Myfile;

    int check;
    char *dirname = "outputs";
    if (access(dirname, F_OK) == -1)
    {
        check = mkdir(dirname, 0777);
    }

    string line = message;

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
    information->threadNumber = threadNumber;

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
            if (buyer->threadNumber != seller->threadNumber)
            {
                Myfile.open("outputs/output" + to_string(buyer->threadNumber) + ".txt", ios::app);
                int t = buyer->expiry + buyer->time - time;
                if (buyer->expiry > 2477288)
                    t = -1;
                Myfile << time << " Kunal "
                       << "SELL " << company << " $" << buyer->price << " #" << tradedQuantity << " " << t << endl;
                Myfile.close();
                Myfile.open("outputs/output" + to_string(seller->threadNumber) + ".txt", ios::app);
                t = seller->expiry + seller->time - time;
                if (seller->expiry > 2477288)
                    t = -1;
                Myfile << time << " Kunal "
                       << "BUY " << company << " $" << seller->price << " #" << tradedQuantity << " " << t << endl;
                Myfile.close();
                netProfit += abs((seller->price - buyer->price) * tradedQuantity);
            }

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
// Function to handle a client connection
void *handleClient(void *arg)
{
    ClientInfo *clientInfo = static_cast<ClientInfo *>(arg);
    char buffer[BUFFER_SIZE];

    std::cout << "Connected to client, IP: " << inet_ntoa(clientInfo->address.sin_addr) << ", Port: " << ntohs(clientInfo->address.sin_port) << std::endl;

    while (true)
    {
        // Receive data from the client
        ssize_t bytesRead = recv(clientInfo->socket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0)
        {
            // Error or connection closed
            if (bytesRead == 0)
            {
                std::cout << "Connection closed by client, IP: " << inet_ntoa(clientInfo->address.sin_addr) << ", Port: " << ntohs(clientInfo->address.sin_port) << std::endl;
            }
            else
            {
                perror("Recv error");
            }
            break;
        }
        else
        {
            // Print the received message
            buffer[bytesRead] = '\0';
            std::cout << "Received message from client, IP: " << inet_ntoa(clientInfo->address.sin_addr) << ", Port: " << ntohs(clientInfo->address.sin_port) << ": " << buffer << std::endl;
            updateTraceFile(buffer, clientInfo->threadNumber);
            lineNo++;
            if (lineNo % NUM_THREADS == 0)
            {
                time1++;
            }
        }
    }

    // Close the client socket
    close(clientInfo->socket);
    delete clientInfo;
    pthread_exit(NULL);
}

int main()
{
    int serverSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    // Create server socket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    // Initialize server address struct
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888); // Port number
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind server socket to the specified address and port
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("Bind error");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(serverSocket, 5) == -1)
    { // Maximum 5 pending connections
        perror("Listen error");
        exit(EXIT_FAILURE);
    }

    std::cout << "Trader is listening on port 8888..." << std::endl;

    std::vector<pthread_t> clientThreads;

    for (int i = 0; i < NUM_THREADS; i++)
    {

        // Accept incoming connections
        int clientSocket;
        if ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen)) == -1)
        {
            perror("Accept error");
            continue; // Continue listening for other connections
        }

        // Create a thread to handle this client
        ClientInfo *clientInfo = new ClientInfo(clientSocket, clientAddr, i);
        pthread_t clientThread;
        if (pthread_create(&clientThread, NULL, handleClient, clientInfo) != 0)
        {
            perror("Thread creation error");
            delete clientInfo;
            continue; // Continue listening for other connections
        }
        // Store the thread ID for later joining
        clientThreads.push_back(clientThread);
    }

    // Join all client threads (clean up)
    for (auto &thread : clientThreads)
    {
        pthread_join(thread, NULL);
    }

    // Close the server socket (never reached in this example)
    close(serverSocket);
endProcess:
    cout << netProfit;
    return 0;
}

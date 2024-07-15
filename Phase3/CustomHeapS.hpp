#ifndef CUSTOM_HEAP_S_HPP
#define CUSTOM_HEAP_S_HPP

#include <vector>
#include <string>
#include "CustomHeap.hpp"

class CustomHeapS
{
private:
    std::vector<info *> data;
    std::string action;

    void swap(info *&a, info *&b)
    {
        info *temp = a;
        a = b;
        b = temp;
    }

    void heapify(int i)
    {
        int largest = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        if (left < data.size() && compare(data[left], data[largest]))
            largest = left;
        if (right < data.size() && compare(data[right], data[largest]))
            largest = right;

        if (largest != i)
        {
            swap(data[i], data[largest]);
            heapify(largest);
        }
    }

    bool compare(const info *a, const info *b)
    {
        if (a->price != b->price)
            return a->price < b->price;
        if (a->time != b->time)
            return a->time > b->time;
        return a->trader > b->trader;
    }

public:
    explicit CustomHeapS() {}

    void push(info *item)
    {
        data.push_back(item);
        int i = data.size() - 1;
        while (i != 0 && compare(data[i], data[(i - 1) / 2]))
        {
            swap(data[i], data[(i - 1) / 2]);
            i = (i - 1) / 2;
        }
    }

    void pop()
    {
        if (data.size() == 0)
            return;

        data[0] = data.back();
        data.pop_back();
        heapify(0);
    }

    info *top()
    {
        if (data.size() == 0)
            return nullptr;
        return data[0];
    }

    bool empty()
    {
        return data.size() == 0;
    }
};

#endif // CUSTOM_HEAP_HPP

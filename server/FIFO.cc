#include "FIFO.h"
void FIFO::push(char c)
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        list.push_back(c);
        if (size_max > 0)while (list.size() > (size_t)size_max) list.pop_front();
    }
    c_v.notify_all();
}

int FIFO::pop(char* c, int size)
{
    std::unique_lock<std::mutex> lock(mutex);
    c_v.wait(lock,[this,size]{return list.size() >= (size_t)size || f(); });
    int i;
    for(i = 0; i < size && 0 < list.size(); ++i)
    {
        c[i] = list.front();
        list.pop_front();
    }
    return i;
}

bool FIFO::is_empty()
{
    return list.size() == 0;
}

void FIFO::notify()
{
    c_v.notify_all();
}

#ifndef FIFO_H
#define FIFO_H

#include <list>
#include <mutex>
#include <condition_variable>
#include <functional>

class FIFO {
private:
    std::list<char> list;
    int size_max;
    std::mutex mutex;
    std::condition_variable c_v;
    std::function<bool()> f;
public:
    FIFO(int size_max, std::function<bool()> f) : size_max(size_max), f(f) {};

    FIFO(std::function<bool()> f) : size_max(-1), f(f) {};

    void push(char c);

    int pop(char *c, int size);

    bool is_empty();

    void notify();
};


#endif //FIFO_H

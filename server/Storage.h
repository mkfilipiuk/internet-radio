#ifndef STORAGE_H
#define STORAGE_H

#include <cstdint>
#include <vector>
#include <list>
#include <condition_variable>
#include "../common/err.h"
#include <arpa/inet.h>

//https://stackoverflow.com/questions/3022552/is-there-any-standard-htonl-like-function-for-64-bits-integers-in-c
#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

struct package
{
    uint64_t session_id;
    uint64_t first_byte_num;
    std::vector<char> audio_data;
    package(uint64_t session_id,
    uint64_t first_byte_num,
    std::vector<char> audio_data) : session_id(session_id), first_byte_num(first_byte_num), audio_data(audio_data) {}
};

class Storage {
private:
    int size;
    std::list<package> packages_to_send;
    std::mutex mutex;
    std::condition_variable c_v;
    bool is_end = false;

public:
    Storage(int size) : size(size)
    {
    }

    void push(uint64_t session_id, uint64_t first_byte_num, std::vector<char> audio_data);

    void push_front(std::list<package>& lp);

    package pop();

    void notify();

    size_t size_of_queue();
};


#endif //STORAGE_H

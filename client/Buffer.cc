//
// Created by mkfil on 09.06.2018.
//

#include <cstring>
#include <sys/time.h>
#include "Buffer.h"

void Buffer::clean() {
    std::unique_lock<std::mutex> lock(mut_send);
    std::unique_lock<std::mutex> lock2(mut_ret);
    to_send.clear();
    to_retransmit.clear();
    first_byte = -1;
    start = false;
}

void Buffer::put(uint64_t session_id, uint64_t no_of_byte, unsigned long length, char *string) {
    std::unique_lock<std::mutex> lock(mut_send);
    if (first_byte == -1) first_byte = no_of_byte;
    if (max_size == -1) max_size = BSIZE / length;
    if (BYTE0 == -1) {
        BYTE0 = no_of_byte;
        counter = BYTE0;
    }
    last_byte = std::max(last_byte, no_of_byte);
    auto iter = to_send.begin();
    while (iter->first < last_byte) {
        to_send.erase(iter++);
        counter += length;
    }
    if (this->session_id != session_id) {
        to_send.clear();
        this->session_id = session_id;
    }
    std::vector<char> v;
    v.assign(string, string + length);
    struct timeval tv;
    gettimeofday(&tv, NULL);
    if (last_byte + length != no_of_byte) {
        std::unique_lock<std::mutex> lock2(mut_ret);
        for (uint64_t s = std::max(last_byte + length, no_of_byte - max_size * length); s < no_of_byte; s += length) {
            to_retransmit.insert(std::make_pair((tv.tv_sec) * 1000 + (tv.tv_usec) / 1000, s));
        }
    }//TODO retransmit
    c_v_ret.notify_one();
    to_send.insert(std::make_pair(no_of_byte, v));

    if ((last_byte - BYTE0) * 4 > max_size * length * 3)c_v_send.notify_one();
}

int Buffer::get(char *string) {
    std::unique_lock<std::mutex> lock(mut_send);
    c_v_send.wait(lock, [this]() { return this->start; });
    if (to_send.find(counter) == to_send.end()) {
        this->clean();
        return -1;
    }
    int i = to_send.begin()->second.size();
    counter += i;
    memcpy(string, to_send.begin()->second.data(), i);
    return i;
}

std::pair<int, uint64_t> Buffer::retransmit() {
    std::unique_lock<std::mutex> lock2(mut_ret);
    c_v_ret.wait(lock2, [this]() { return to_retransmit.size() != 0; });
    std::unique_lock<std::mutex> lock(mut_send);
    auto a = to_retransmit.begin();
    while (true) {
        a = to_retransmit.begin();
        if (to_send.find(a->second) != to_send.end() || a->second < last_byte - BSIZE) {
            to_retransmit.erase(a);
        } else break;
    }
    return *a;
}

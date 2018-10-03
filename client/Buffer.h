//
// Created by mkfil on 09.06.2018.
//

#ifndef SIK_BUFFER_H
#define SIK_BUFFER_H


#include <cstdint>
#include <vector>
#include <map>
#include <set>
#include <mutex>
#include <condition_variable>
#include "menu.h"

class Buffer {
    uint64_t first_byte = -1, last_byte, BYTE0 = -1, session_id = -1, counter;
    int RTIME;
    bool start = false;
    int BSIZE, max_size = -1;
    std::mutex mut_send, mut_ret;
    std::condition_variable c_v_send, c_v_ret;
    std::map<uint64_t, std::vector<char>> to_send;
    std::set<std::pair<uint64_t, uint64_t>> to_retransmit;
    Menu *menu;
public:
    Buffer(int BSIZE, Menu *menu, int RTIME) : BSIZE(BSIZE), last_byte(0), menu(menu), RTIME(RTIME) {}

    void clean();

    void put(uint64_t session_id, uint64_t no_of_byte, unsigned long length, char string[65536]);

    int get(char string[65536]);

    std::pair<int, uint64_t> retransmit();
};


#endif //SIK_BUFFER_H

//
// Created by mkfil on 31.05.2018.
//

#include "Broadcaster.h"

bool Broadcaster::is_alive_and_tick(bool is_alive) {
    ++time_from_last_tick;
    if (is_alive) time_from_last_tick = 0;
    if (time_from_last_tick == no_of_missed_before_being_deleted) return false;
    return true;
}

std::string Broadcaster::get_name() {
    return name;
}

void Broadcaster::tick() {
    time_from_last_tick = 0;
}

void Broadcaster::tickup() const {
    time_from_last_tick++;
}

bool Broadcaster::is_alive() const {
    return time_from_last_tick < no_of_missed_before_being_deleted;
}

int Broadcaster::get_tick() {
    return time_from_last_tick;
}

void Broadcaster::clear() {
    name = "";
    port = 0;
    address = 0;
    session_id = (uint64_t) -1;
    time_from_last_tick = 0;
    sender = {};
    sendsize = 0;
}

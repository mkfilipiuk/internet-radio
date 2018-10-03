//
// Created by mkfil on 31.05.2018.
//

#ifndef BROADCASTER_H
#define BROADCASTER_H


#include <string>
#include <sys/socket.h>

class Broadcaster {
private:
    std::string name;
    unsigned short port;
    unsigned long address;
    uint64_t session_id = (uint64_t) -1;
    mutable int time_from_last_tick = 0;
    const int no_of_missed_before_being_deleted = 4;
    struct sockaddr_storage sender;
    socklen_t sendsize;

public:
    Broadcaster(std::string name, unsigned short port, unsigned long address, struct sockaddr_storage sender,
                socklen_t sendsize) : name(name), port(port), address(address), sender(sender), sendsize(sendsize) {}

    Broadcaster(const Broadcaster &b) : name(b.name), port(b.port), address(b.address), sender(b.sender),
                                        sendsize(b.sendsize), session_id(b.session_id) {}

    void set_session_id(uint64_t session) {
        this->session_id = session;
    }

    bool is_alive_and_tick(bool is_alive);

    bool operator<(Broadcaster other) const {
        return name < other.name;
    }


    std::string get_name();

    void tick();

    bool is_alive() const;

    void tickup() const;

    int get_tick();

    unsigned long get_address() { return address; }

    unsigned long get_port() { return port; }

    struct sockaddr_storage get_sender() { return sender; };

    socklen_t get_sendsize() { return sendsize; };

    inline bool operator==(const Broadcaster &lb) const { return lb.port == this->port && lb.address == this->address &&
                                                                 ((lb.session_id != (uint64_t) -1 &&
                                                                   this->session_id != (uint64_t) -1) ? lb.session_id ==
                                                                                                        this->session_id
                                                                                                      : true);
    }

    Broadcaster &operator=(const Broadcaster &other) {
        name = other.name;
        port = other.port;
        address = other.address;
        session_id = other.session_id;
        time_from_last_tick = other.time_from_last_tick;
        sender = other.sender;
        sendsize = other.sendsize;
        return *this;
    }

    void clear();
};


#endif //ZADANIE2_SIK_BROADCASTER_H

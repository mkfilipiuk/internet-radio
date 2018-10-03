#ifndef MENU_H
#define MENU_H


#include <string>
#include <set>
#include <mutex>
#include <sys/socket.h>
#include <condition_variable>
#include <algorithm>
#include <sstream>
#include "Broadcaster.h"

enum action_e {
    ARROW_UP,
    ARROW_DOWN,
    NONE
};

class Menu {
private:
    int time = 0;
    std::set<Broadcaster> active_broadcasters;
    std::set<std::string> ticking_broadcasters;
    std::mutex m;
    int active = 0;
    Broadcaster active_e;
    std::string NAZWA;
    std::condition_variable *sleeping_main;

public:
    void tick();

    std::string list_broadcasters();

    void move_cursor(action_e a_e);

    Menu(std::string NAZWA, std::condition_variable *sleeping_main) : NAZWA(NAZWA), sleeping_main(sleeping_main),
                                                                      active_e("", 0, 0, {}, 0) {
    }

    Broadcaster get_active();

    void add_ticking(std::string s, unsigned long address, unsigned short port, struct sockaddr_storage sender,
                     socklen_t sendsize);
};


#endif //MENU_H

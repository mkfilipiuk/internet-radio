

#include "menu.h"

void Menu::tick() {
    std::unique_lock<std::mutex> lock(m);
    int i = active_broadcasters.size();
    for (auto iter = active_broadcasters.begin(); iter != active_broadcasters.end(); ++iter) { iter->tickup(); }
    for (auto iter = active_broadcasters.begin(); iter != active_broadcasters.end();) {
        if (!iter->is_alive()) {
            active_broadcasters.erase(iter++);
        } else {
            ++iter;
        }
    }
    if (active_broadcasters.size() == 0) {
        active_e.clear();
        active = 0;
    }
}

void Menu::add_ticking(std::string s, unsigned long address, unsigned short port, struct sockaddr_storage sender,
                       socklen_t sendsize) {
    std::unique_lock<std::mutex> lock(m);
    auto b = Broadcaster(s, port, address, sender, sendsize);
    if (active_broadcasters.find(b) == active_broadcasters.end()) {
        active_broadcasters.insert(b);
        if (b.get_name().compare(s) == 0) {
            active_e = b;
            active = [this] {
                auto a = active_broadcasters.find(active_e);
                auto b = active_broadcasters.begin();
                int i = 0;
                while (b != a) {
                    b++;
                    i++;
                }
                return i;
            }();
        }
        if (active_broadcasters.size() == 1) {
            active_e = b;
            sleeping_main->notify_one();
            active = 0;
        }
    } else {
        Broadcaster bb = *active_broadcasters.find(b);
        active_broadcasters.erase(active_broadcasters.find(b));
        bb.tick();
        active_broadcasters.insert(bb);
    }

}


std::string Menu::list_broadcasters() {
    std::unique_lock<std::mutex> lock(m);
    std::stringstream ss;
    int i = 0;
    ss << "\r";
    for (auto b : active_broadcasters) {
        if (i == active) ss << "   > ";
        else ss << "     ";
        ss << b.get_name() << std::endl << "\r";
        ++i;
    }
    return ss.str();
}


void Menu::move_cursor(action_e a_e) {
    fprintf(stderr, "Otrzymalismy rozkaz %d\n", a_e);
    if (a_e == ARROW_UP) active = std::max(active - 1, 0);
    if (a_e == ARROW_DOWN) active = std::min((int) active_broadcasters.size() - 1, active + 1);
}

Broadcaster Menu::get_active() {
    return active_e;
}

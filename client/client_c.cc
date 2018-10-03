#include "client_c.h"
#include "Broadcaster.h"
#include <memory>
#include <thread>
#include <poll.h>
#include <cstring>
#include <vector>
#include <sstream>
#include <climits>
#include <arpa/inet.h>

const int TTL_VALUE = 4;

#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

void Client::clear_buffer() {
    for (size_t i = 0; i < buffer_size; ++i)client_buffer[i] = 0;
}

void Client::prepare_sockets() {
    ui_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (ui_sock < 0) syserr("socket");
    ctrl_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (ctrl_sock < 0) syserr("socket");
    data_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (data_sock < 0)
        syserr("socket");


    int optval = 1;
    if (setsockopt(ctrl_sock, SOL_SOCKET, SO_BROADCAST, (void *) &optval, sizeof optval) < 0)
        syserr("setsockopt broadcast");
    optval = TTL_VALUE;
    if (setsockopt(ctrl_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void *) &optval, sizeof optval) < 0)
        syserr("setsockopt multicast ttl");
    optval = 0;
    if (setsockopt(ctrl_sock, SOL_IP, IP_MULTICAST_LOOP, (void *) &optval, sizeof optval) < 0)
        syserr("setsockopt loop");
    struct timeval tv2;
    tv2.tv_sec = 1;
    tv2.tv_usec = 0;
    setsockopt(ctrl_sock, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv2, sizeof tv2);

    ctrl_address.sin_family = AF_INET;
    ctrl_address.sin_addr = DISCOVER_ADDR;
    ctrl_address.sin_port = htons(CTRL_PORT);
    UI_address.sin_family = AF_INET; // IPv4
    UI_address.sin_addr.s_addr = htonl(INADDR_ANY); // listening on all interfaces
    UI_address.sin_port = htons(UI_PORT); // listening on port PORT_NUM
    if (bind(ui_sock, (struct sockaddr *) &UI_address, sizeof(UI_address)) < 0) syserr("bind");
    if (listen(ui_sock, queue_lenght) < 0) syserr("listen");

}

void Client::configure_telnet() {
    unsigned char conf_telnet[3] = {255, 253, 34};
    int len = 3;
    if (write(connected_ui_sock, conf_telnet, len) != len)syserr("Telnet config");
    len = 7;
    unsigned char conf_telnet2[7] = {255, 250, 34, 1, 0, 255, 240};
    if (write(connected_ui_sock, conf_telnet2, len) != len)syserr("Telnet config");
    unsigned char conf_telnet25[3] = {255, 251, 1};
    len = 3;
    if (write(connected_ui_sock, conf_telnet25, len) != len)syserr("Telnet config");
    unsigned char conf_telnet3[8] = "\e[?25l\r";
    len = 8;
    if (write(connected_ui_sock, conf_telnet3, len) != len)syserr("Telnet config");
}


void Client::write_to_client() {
    snd_len = write(connected_ui_sock, client_buffer, len);
    if (snd_len != len) {
        fprintf(stderr, "We have problem with connection\n");
        len = 0;
    }
}


void Client::run() {
    prepare_sockets();
    std::thread thread_broadcast([this]() { this->telnet_relation(); });
    std::thread thread_identify([this]() { this->identify_broadcasters(); });
    std::thread thread_play([this]() { this->play(); });
    std::thread thread_retransmit([this]() { this->retransmit(); });

    while (true) {
        std::unique_lock<std::mutex> lk(mutex);
        sleeping_main.wait(lk, [this]() { return menu.get_active().get_address() != 0; });
        Broadcaster broadcaster(menu.get_active());
        bool is_socket_configured = false;
        buffer_out.clean();
        while (broadcaster == menu.get_active()) {
            if (!is_socket_configured) {
                ip_data.imr_interface.s_addr = htonl(INADDR_ANY);
                ip_data.imr_multiaddr.s_addr = broadcaster.get_address();
                if (setsockopt(data_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *) &ip_data, sizeof(ip_data)) < 0)
                    syserr("setsockopt 2137");
                ip_data.imr_interface.s_addr = htonl(INADDR_ANY);
                data_address.sin_family = AF_INET;
                data_address.sin_addr.s_addr = htonl(INADDR_ANY);
                data_address.sin_port = htons(broadcaster.get_port());
                if (bind(data_sock, (struct sockaddr *) &data_address, sizeof data_address) < 0)
                    syserr("bind");
                is_socket_configured = true;
            }
            int rcv_len = read(data_sock, data_arr, data_arr_size);

            buffer_out.put(ntohll(((uint64_t *) data_arr)[0]), ntohll(((uint64_t *) data_arr)[1]),
                           rcv_len - 2 * sizeof(uint64_t), data_arr + 2 * sizeof(uint64_t));
        }
        buffer_out.clean();
        if (setsockopt(data_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (void *) &ip_data, sizeof(ip_data)))
            syserr("setsockopt");
        close(data_sock);
        is_socket_configured = false;
    }
    thread_play.join();
    thread_retransmit.join();
    thread_broadcast.join();
    thread_identify.join();
}


void Client::write_message_to_client(std::string message) {
    clear_buffer();
    size_t i;
    for (i = 0; i < message.size(); ++i) client_buffer[i] = message[i];
    client_buffer[i] = '\n';
    ++i;
    size_t j = 0;
    if (chosen_option.compare("") != 0) {
        for (j = 0; j < chosen_option.size(); ++j) client_buffer[i + j] = chosen_option[j];
        client_buffer[i + j] = '\n';
        ++j;
    }
    len = i + j;
    write_to_client();
}


action_e Client::analize_input() {
    char b;
    int state = 0;
    std::vector<char> v;
    while (1) {
        if (buffer_l.size() == 0) {
            for (char c : v)buffer_l.push_back(c);
            break;
        }
        b = buffer_l.front();
        v.push_back(b);
        buffer_l.pop_front();
        if (state == 0) {
            switch (b) {
                case '\033':
                    state = 2;
                    break;
                default:
                    break;
            }
            continue;
        }

        if (state == 2) {
            if (b == '[') state = 3;
            else state = 0;
            continue;
        }

        if (state == 3) {
            if (b == 'A') return ARROW_UP;
            if (b == 'B') return ARROW_DOWN;
            state = 0;
        }
    }
    return NONE;
}

void Client::telnet_relation() {
    struct pollfd sockets[_POSIX_OPEN_MAX];
    for (int i = 0; i < _POSIX_OPEN_MAX; ++i) {
        sockets[i].fd = -1;
        sockets[i].events = POLLIN;
        sockets[i].revents = 0;
    }
    struct pollfd client_poll;
    client_poll.fd = ui_sock;
    client_poll.events = POLLIN;
    client_poll.revents = 0;
    sockets[0] = client_poll;
    int activeClients = 0;
    int i, rval;
    while (true) {
        for (int i = 0; i < _POSIX_OPEN_MAX; ++i) sockets[i].revents = 0;
        int ret = poll(sockets, _POSIX_OPEN_MAX, 5000);
        if (ret < 0)
            perror("poll");
        else if (ret > 0) {
            if (sockets[0].revents & POLLIN) { //new connection
                int msgsock = accept(sockets[0].fd, (struct sockaddr *) 0, (socklen_t *) 0);
                connected_ui_sock = msgsock;
                configure_telnet();
                if (msgsock == -1)
                    perror("accept");
                else {
                    for (i = 1; i < _POSIX_OPEN_MAX; ++i) {
                        if (sockets[i].fd == -1) {
                            sockets[i].fd = msgsock;
                            activeClients += 1;
                            break;
                        }
                    }
                    if (i >= _POSIX_OPEN_MAX) {
                        if (close(msgsock) < 0)
                            perror("close");
                    }
                }
            }
            for (i = 1; i < _POSIX_OPEN_MAX; ++i) {
                if (sockets[i].fd != -1 && (sockets[i].revents & (POLLIN | POLLERR))) {
                    rval = read(sockets[i].fd, client_buffer, buffer_size);
                    if (rval < 0) {
                        perror("Reading stream message");
                        if (close(sockets[i].fd) < 0)
                            perror("close");
                        sockets[i].fd = -1;
                        activeClients -= 1;
                    } else if (rval == 0) {
                        if (close(sockets[i].fd) < 0)
                            perror("close");
                        sockets[i].fd = -1;
                        activeClients -= 1;
                    } else {
                        for (int i = 0; i < rval; ++i) buffer_l.push_back(client_buffer[i]);
                        action_e a_e;
                        while ((a_e = analize_input()) != NONE) {
                            menu.move_cursor(a_e);
                        }
                    }
                }
            }
        }
        std::string m = get_menu();
        for (i = 1; i < _POSIX_OPEN_MAX; ++i) {
            if (sockets[i].fd != -1) {
                connected_ui_sock = sockets[i].fd;
                write_message_to_client(m);
            }
        }
    }
}

std::string Client::get_menu() {
    std::stringstream ss;
    ss << "\u001B[";
    ss << "\033[2J";
    ss << "\033[H";
    ss << "------------------------------------------------------------------------" << std::endl << "\r";
    ss << "  " << "SIK Radio" << std::endl << "\r";
    ss << "------------------------------------------------------------------------" << std::endl << "\r";
    ss << menu.list_broadcasters();
    ss << "------------------------------------------------------------------------" << std::endl << "\r";
    return ss.str();
}

void Client::identify_broadcasters() {
    while (true) {
        sprintf(discover_buffer, "ZERO_SEVEN_COME_IN\n");
        int len = std::string("ZERO_SEVEN_COME_IN\n").size();
        sendto(ctrl_sock, discover_buffer, len, 0, (struct sockaddr *) &ctrl_address, sizeof ctrl_address);
        struct sockaddr_storage sender;
        socklen_t sendsize = sizeof(sender);
        memset(&sender, 0, sendsize);
        while ((len = recvfrom(ctrl_sock, discover_buffer, buffer_size, 0, (struct sockaddr *) &sender, &sendsize)) >
               0) {
            int i = 0;
            while (discover_buffer[i++] != ' ') {}
            int size = 0;
            while (discover_buffer[i + size] != ' ') { size++; }
            std::string MCAST, PORT, nazwa;

            MCAST = std::string(discover_buffer).substr(i, size);
            i = i + size + 1;
            size = 0;
            while (discover_buffer[i + size] != ' ') { size++; }
            PORT = std::string(discover_buffer).substr(i, size);

            i = i + size + 1;
            nazwa = std::string(discover_buffer).substr(i, std::string(discover_buffer).size() - i);
            unsigned long l;
            if (inet_pton(AF_INET, MCAST.c_str(), &l) == 0)
                syserr("inet_pton");
            menu.add_ticking(nazwa, l, stoi(PORT), sender, sendsize);
        }
        menu.tick();
        std::this_thread::sleep_for(std::chrono::seconds(lookup_interval));
    }
}

void Client::play() {
    char print[65536];
    int i;
    while (true) {
        i = buffer_out.get(print);
        if (i == -1) continue;
        fwrite(print, sizeof(char), i, stdout);
    }
}

void Client::retransmit() {
    while (true) {
        auto p = buffer_out.retransmit();

    }
}



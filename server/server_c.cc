#include "server_c.h"
#include "cyclic_buffer.h"
#include <memory>
#include <thread>
#include <chrono>
#include <fcntl.h>
#include <csignal>
#include <regex>
#include <poll.h>
#include <asm/ioctls.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <iostream>
#include <set>
#include <sys/time.h>

const int TTL_VALUE = 4; //Długość życia pakietu


void Server::prepare_sockets() {
    data_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (data_sock < 0) syserr("socket");
    int optval = 1;
    if (setsockopt(data_sock, SOL_SOCKET, SO_BROADCAST, (void *) &optval, sizeof optval) < 0)
        syserr("setsockopt broadcast");
    optval = TTL_VALUE;
    if (setsockopt(data_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void *) &optval, sizeof optval) < 0)
        syserr("setsockopt multicast ttl");
    optval = 0;
    if (setsockopt(data_sock, SOL_IP, IP_MULTICAST_LOOP, (void *) &optval, sizeof optval) < 0)
        syserr("setsockopt loop");
    server_address_data.sin_family = AF_INET; // IPv4
    server_address_data.sin_addr = MCAST_ADDR;
    server_address_data.sin_port = htons(DATA_PORT);
    if (connect(data_sock, (struct sockaddr *) &server_address_data, sizeof(server_address_data)) < 0)
        syserr("connect");

    ctrl_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (ctrl_sock < 0) syserr("socket");
    memset(&client_address_ctrl, 0, sizeof(client_address_ctrl));
    int on = 1;
    if (setsockopt(ctrl_sock, SOL_SOCKET, SO_REUSEADDR,
                   (char *) &on, sizeof(on)) < 0) {
        perror("setsockopt() failed");
        close(ctrl_sock);
        exit(-1);
    }
    client_address_ctrl.sin_family = AF_INET;
    client_address_ctrl.sin_addr.s_addr = htonl(INADDR_ANY);
    client_address_ctrl.sin_port = htons(CTRL_PORT);
    if (bind(ctrl_sock, (struct sockaddr *) &client_address_ctrl, sizeof(client_address_ctrl)) < 0) syserr("bind");

    if (ioctl(ctrl_sock, FIONBIO, (char *) &on) < 0) {
        perror("ioctl() failed");
        close(ctrl_sock);
        exit(-1);
    }
}

void Server::broadcast() {
    int array_size = sizeof(char) * PSIZE + 2 * sizeof(uint64_t);
    char *array = new char[array_size];
    while (!is_end) {
        package i = fifo.pop();
        if (i.audio_data.size() == 0) break;
        memcpy(array, (char *) &i.session_id, sizeof(uint64_t));
        memcpy(array + sizeof(uint64_t), (char *) &i.first_byte_num, sizeof(uint64_t));
        memcpy(array + 2 * sizeof(uint64_t), i.audio_data.data(), PSIZE);

        if (write(data_sock, array, array_size) != array_size)
            syserr("write");
    }
    delete[] array;
}


void Server::listen_from_clients() {
    struct pollfd client_poll;
    client_poll.fd = ctrl_sock;
    client_poll.events = POLLIN;
    client_poll.revents = 0;
    while (!is_end) {
        client_poll.revents = 0;
        int ret = poll(&client_poll, 1, 1000);
        if (ret < -1) {
            perror("poll");
            return;
        }
        if (ret == 1 && client_poll.revents & POLLIN) {
            memset(client_buffer, 0, client_buffer_size);
            struct sockaddr_storage sender;
            socklen_t sendsize = sizeof(sender);
            bzero(&sender, sizeof(sender));
            if (recvfrom(ctrl_sock, client_buffer, client_buffer_size, 0, (struct sockaddr *) &sender, &sendsize) ==
                -1)
                syserr("recv");
            std::string s(client_buffer);
            std::regex inst1("^ZERO_SEVEN_COME_IN\n$");
            std::regex inst2("^LOUDER_PLEASE (\\d+,)*\\d+\n$");
            if (std::regex_match(s, inst1)) {
                int ss = snprintf(client_buffer, client_buffer_size, "BOREWICZ_HERE %s %d %s\n",
                                  MCAST_ADDR_s.c_str(), DATA_PORT, NAZWA.c_str());
                sendto(ctrl_sock, client_buffer, ss, 0, (struct sockaddr *) &sender, sendsize);
            } else if (std::regex_match(s, inst2)) {
                std::replace(s.begin(), s.end(), ',', ' ');
                std::istringstream is(s);
                int n;
                std::string b;
                is >> b;
                std::set<uint64_t> s;
                while (is >> n) {
                    s.insert(n);
                }
                for (auto i = s.begin(); i != s.end(); ++i) {
                    auto it = fifo_storage.find(*i);
                    if (it != fifo_storage.end()) {
                        retransmits.push(htonll(it->second.session_id),
                                         htonll(it->second.first_byte_num),
                                         it->second.audio_data);
                    }
                }
            }
        }
    }
}

void Server::retransmit() {
    std::mutex rtime_m;
    std::unique_lock<std::mutex> lk(rtime_m);
    while (!is_end) {
        struct timeval tv, tv2;
        gettimeofday(&tv, NULL);

        std::list<package> l;
        int n = retransmits.size_of_queue();
        while (n > 0) {
            l.push_back(retransmits.pop());
            --n;
        }
        fifo.push_front(l);
        gettimeofday(&tv2, NULL);
        uint64_t t = std::max((uint64_t) (RTIME - (tv2.tv_usec - tv.tv_usec + 1000000 * (tv2.tv_sec - tv.tv_sec))),
                              (uint64_t) 0);
        c_v_RTIME.wait_for(lk, std::chrono::milliseconds(t), [this] { return is_end; });
    }
}

void Server::run() {
    prepare_sockets();
    std::thread thread_broadcast([this]() { this->broadcast(); });
    std::thread thread_responder([this]() { this->listen_from_clients(); });
    std::thread thread_retransmit([this]() { this->retransmit(); });
    std::vector<char> v;
    uint64_t no_of_bytes = 0;
    freopen(NULL, "rb", stdin);
    char *buffor_stdin = new char[PSIZE];
    int readed = 0;
    while (true) {
        if ((readed = read(0, buffor_stdin, PSIZE)) == 0) break;
        cyclic_buffer c_b(PSIZE);
        c_b.push(buffor_stdin, readed);

        while (c_b.size() >= (size_t)PSIZE) {
            std::vector<char> vv = c_b.pop();
            no_of_bytes += readed;
            fifo.push(session_id, no_of_bytes, vv);
            fifo_storage.insert(std::make_pair(no_of_bytes - PSIZE, package(session_id, no_of_bytes - PSIZE, vv)));
            if ((size_t)fifo_storage.size() > (size_t)fifo_storage_size) fifo_storage.erase(fifo_storage.begin()->first);
        }
    }
    is_end = true;
    delete buffor_stdin;
    fifo.notify();
    thread_responder.join();
    c_v_RTIME.notify_one();
    thread_retransmit.join();
    thread_broadcast.join();
    close_server();
}

void Server::close_server() {
    close(ctrl_sock);
    close(data_sock);
}



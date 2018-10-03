#ifndef SIK_SERVER
#define SIK_SERVER

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdio>
#include <string>
#include <unistd.h>
#include <memory>
#include <map>
#include <list>
#include <set>
#include "../common/err.h"
#include "menu.h"
#include "Buffer.h"


class Client {
private:
    struct in_addr DISCOVER_ADDR;
    unsigned short CTRL_PORT;
    unsigned short UI_PORT;
    unsigned int PSIZE;
    unsigned int BSIZE;
    unsigned int RTIME;
    std::string NAZWA;
    std::condition_variable sleeping_main;
    std::mutex mutex;
    Buffer buffer_out;
    struct sockaddr_in data_address;
    struct ip_mreq ip_data;
    const static int data_arr_size = 65536;
    char data_arr[data_arr_size];

    const int lookup_interval = 5;
    const int wait_for_broadcasters = 1;


    int ctrl_sock, data_sock, ui_sock, connected_ui_sock;

    Menu menu;


    static const int buffer_size = 10000;
    static const int queue_lenght = 5;
    char client_buffer[buffer_size], discover_buffer[buffer_size];
    char add_buf[3];
    std::string chosen_option = "";

    struct sockaddr_in UI_address, ctrl_address;

    socklen_t client_address_len;
    ssize_t len, snd_len;
    bool is_connected = false;
    std::list<char> buffer_l;

    action_e analize_input();

    void configure_telnet();

    void write_to_client();

    void clear_buffer();

public:
    Client(
            struct in_addr DISCOVER_ADDR,
            unsigned short CTRL_PORT,
            unsigned short UI_PORT,
            unsigned int PSIZE,
            unsigned int BSIZE,
            unsigned int RTIME,
            std::string NAZWA) :
            DISCOVER_ADDR(DISCOVER_ADDR),
            CTRL_PORT(CTRL_PORT),
            UI_PORT(UI_PORT),
            PSIZE(PSIZE),
            BSIZE(BSIZE),
            RTIME(RTIME),
            NAZWA(NAZWA),
            menu(NAZWA, &sleeping_main),
            buffer_out(BSIZE, &menu, RTIME) {}

    void set_port(int port);

    void prepare_sockets();

    void bind_socket();

    void listen_on_socket();

    void run();

    void write_message_to_client(std::string message);

    void close_server();

    void telnet_relation();

    std::string get_menu();

    void identify_broadcasters();

    void play();

    void retransmit();
};

#endif 

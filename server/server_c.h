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
#include <mutex>
#include <condition_variable>
#include "../common/err.h"
#include "FIFO.h"
#include "Storage.h"

class Server {
private:
    static const int client_buffer_size = 1000;
    static const int queue_lenght = 5;
    struct in_addr MCAST_ADDR;
    std::string MCAST_ADDR_s;
    unsigned short DATA_PORT;
    unsigned short CTRL_PORT;
    unsigned int PSIZE;
    unsigned int BSIZE;
    unsigned int FSIZE;
    unsigned int RTIME;
    std::string NAZWA;
    Storage fifo, retransmits;
    std::map<uint64_t, package> fifo_storage;
    int fifo_storage_size = FSIZE / PSIZE;
    bool is_end = false;
    int data_sock, ctrl_sock;
    char client_buffer[client_buffer_size];
    uint64_t session_id = static_cast<uint64_t>(time(0));
    std::condition_variable c_v_RTIME;

    struct sockaddr_in server_address_data;
    struct sockaddr_in client_address_ctrl;
    socklen_t client_address_len;
    ssize_t len, snd_len;

    void listen_from_clients();

    void prepare_sockets();


    void broadcast();

    void respond();

public:
    Server(struct in_addr MCAST_ADDR,
           std::string MCAST_ADDR_s,
           unsigned short DATA_PORT,
           unsigned short CTRL_PORT,
           unsigned int PSIZE,
           unsigned int BSIZE,
           unsigned int FSIZE,
           unsigned int RTIME,
           std::string NAZWA) :
            MCAST_ADDR(MCAST_ADDR),
            MCAST_ADDR_s(MCAST_ADDR_s),
            DATA_PORT(DATA_PORT),
            CTRL_PORT(CTRL_PORT),
            PSIZE(PSIZE),
            BSIZE(BSIZE),
            FSIZE(FSIZE),
            RTIME(RTIME),
            NAZWA(NAZWA),
            fifo(PSIZE),
            retransmits(PSIZE) {}

    bool get_is_end() { return is_end; }

    void run();

    void close_server();

    void retransmit();
};

#endif 

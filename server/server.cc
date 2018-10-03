#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include "../common/err.h"
#include "server_c.h"
#include <netinet/in.h>
#include <arpa/inet.h>


using namespace std;

const int INDEX_NUMBER = 385423;

Server *server;
struct in_addr MCAST_ADDR;
std::string MCAST_ADDR_s;
unsigned short DATA_PORT;
unsigned short CTRL_PORT;
unsigned int PSIZE;
unsigned int BSIZE;
unsigned int FSIZE;
unsigned int RTIME;
string NAZWA;
bool is_MCAST_ADDR = false;
bool is_DATA_PORT = false;
bool is_CTRL_PORT = false;
bool is_PSIZE = false;
bool is_BSIZE = false;
bool is_FSIZE = false;
bool is_RTIME = false;
bool is_NAZWA = false;

void init() {
    inet_pton(AF_INET, "0.0.0.0", &MCAST_ADDR);
    DATA_PORT = 20000 + (INDEX_NUMBER % 10000);
    CTRL_PORT = 30000 + (INDEX_NUMBER % 10000);
    PSIZE = 512;
    BSIZE = 65536;
    FSIZE = 128 * 1024;
    RTIME = 250;
    NAZWA = string("Nienazwany Nadajnik").c_str();
}

int sanitize_port(char *a) {
    int i = 0;
    while (a[i] != 0) {
        if (!(a[i] >= '0' && a[i] <= '9'))return 1;
        ++i;
    }
    return 0;
}


void analyse_input(int argc, char *argv[]) {
    int i = 1;
    if (argc % 2 == 0) syserr("Wrong number of arguments");
    while (i < argc) {
        if (strcmp(argv[i], "-a") == 0) {
            if (is_MCAST_ADDR) syserr("Double declaration of MCAST_ADDR");
            is_MCAST_ADDR = true;
            ++i;
            if (inet_pton(AF_INET, argv[i], &(MCAST_ADDR)) != 1) syserr("Cannot parse MCAST_ADDR");
            MCAST_ADDR_s = argv[i];
            ++i;
            continue;
        }
        if (strcmp(argv[i], "-P") == 0) {
            if (is_DATA_PORT) syserr("Double declaration of DATA_PORT");
            is_DATA_PORT = true;
            ++i;
            if (sanitize_port(argv[i])) syserr("Cannot parse DATA_PORT");
            int tmp = stoi(argv[i]);
            if (tmp < 0 || tmp > 65535) syserr("Cannot parse DATA_PORT");
            DATA_PORT = static_cast<unsigned short>(tmp);
            ++i;
            continue;
        }
        if (strcmp(argv[i], "-C") == 0) {
            if (is_CTRL_PORT) syserr("Double declaration of CTRL_PORT");
            is_CTRL_PORT = true;
            ++i;
            if (sanitize_port(argv[i])) syserr("Cannot parse CTRL_PORT");
            int tmp = stoi(argv[i]);
            if (tmp < 0 || tmp > 65535) syserr("Cannot parse CTRL_PORT");
            CTRL_PORT = static_cast<unsigned short>(tmp);
            ++i;
            continue;
        }
        if (strcmp(argv[i], "-p") == 0) {
            if (is_PSIZE) syserr("Double declaration of PSIZE");
            is_PSIZE = true;
            ++i;
            if (sanitize_port(argv[i])) syserr("Cannot parse PSIZE");
            int tmp = stoi(argv[i]);
            if (tmp <= 0) syserr("Cannot parse PSIZE");
            PSIZE = static_cast<unsigned int>(tmp);
            ++i;
            continue;
        }
        if (strcmp(argv[i], "-b") == 0) {
            if (is_BSIZE) syserr("Double declaration of BSIZE");
            is_BSIZE = true;
            ++i;
            if (sanitize_port(argv[i])) syserr("Cannot parse BSIZE");
            int tmp = stoi(argv[i]);
            if (tmp <= 0) syserr("Cannot parse BSIZE");
            BSIZE = static_cast<unsigned int>(tmp);
            ++i;
            continue;
        }
        if (strcmp(argv[i], "-f") == 0) {
            if (is_FSIZE) syserr("Double declaration of FSIZE");
            is_FSIZE = true;
            ++i;
            if (sanitize_port(argv[i])) syserr("Cannot parse FSIZE");
            int tmp = stoi(argv[i]);
            if (tmp <= 0) syserr("Cannot parse FSIZE");
            FSIZE = static_cast<unsigned int>(tmp);
            ++i;
            continue;
        }
        if (strcmp(argv[i], "-R") == 0) {
            if (is_RTIME) syserr("Double declaration of RTIME");
            is_RTIME = true;
            ++i;
            if (sanitize_port(argv[i])) syserr("Cannot parse RTIME");
            int tmp = stoi(argv[i]);
            if (tmp <= 0) syserr("Cannot parse RTIME");
            RTIME = static_cast<unsigned int>(tmp);
            ++i;
            continue;
        }
        if (strcmp(argv[i], "-n") == 0) {
            if (is_NAZWA) syserr("Double declaration of NAZWA");
            is_NAZWA = true;
            ++i;
            NAZWA = string(argv[i]);
            if (NAZWA.size() > 64) syserr("NAZWA is too long");
            ++i;
            continue;
        }
        syserr("Unrecognisable argument");
    }
    if (!is_MCAST_ADDR)syserr("MCAST_ADDR has to be set");
}

int main(int argc, char *argv[]) {
    init();
    analyse_input(argc, argv);
    server = new Server(MCAST_ADDR, MCAST_ADDR_s, DATA_PORT, CTRL_PORT, PSIZE, BSIZE, FSIZE, RTIME, NAZWA);
    server->run();
    server->close_server();
    delete server;
    return 0;
}

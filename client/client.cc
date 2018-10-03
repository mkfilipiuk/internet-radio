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
#include "client_c.h"
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

const int INDEX_NUMBER = 385423;

Client *client;
struct in_addr DISCOVER_ADDR;
unsigned short CTRL_PORT;
unsigned short UI_PORT;
unsigned int PSIZE;
unsigned int BSIZE;
unsigned int RTIME;
string NAZWA;
bool is_DISCOVER_ADDR = false;
bool is_CTRL_PORT = false;
bool is_UI_PORT = false;
bool is_PSIZE = false;
bool is_BSIZE = false;
bool is_RTIME = false;
bool is_NAZWA = false;

void init() {
    inet_pton(AF_INET, "255.255.255.255", &DISCOVER_ADDR);
    CTRL_PORT = 30000 + (INDEX_NUMBER % 10000);
    UI_PORT = 10000 + (INDEX_NUMBER % 10000);
    PSIZE = 512;
    BSIZE = 65536;
    RTIME = 250;
    NAZWA = string();
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
        if (strcmp(argv[i], "-d") == 0) {
            if (is_DISCOVER_ADDR) syserr("Double declaration of DISCOVER_ADDR");
            is_DISCOVER_ADDR = true;
            ++i;
            if (inet_pton(AF_INET, argv[i], &(DISCOVER_ADDR)) != 1) syserr("Cannot parse DISCOVER_ADDR");
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
        if (strcmp(argv[i], "-U") == 0) {
            if (is_UI_PORT) syserr("Double declaration of UI_PORT");
            is_UI_PORT = true;
            ++i;
            if (sanitize_port(argv[i])) syserr("Cannot parse UI_PORT");
            int tmp = stoi(argv[i]);
            if (tmp < 0 || tmp > 65535) syserr("Cannot parse UI_PORT");
            UI_PORT = static_cast<unsigned short>(tmp);
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
            ++i;
            continue;
        }
        syserr("Unrecognisable argument");
    }
}

int main(int argc, char *argv[]) {
    init();
    analyse_input(argc, argv);
    client = new Client(DISCOVER_ADDR, CTRL_PORT, UI_PORT, PSIZE, BSIZE, RTIME, NAZWA);
    client->run();
    delete client;
    return 0;
}

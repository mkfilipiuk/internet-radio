//
// Created by mkfil on 08.06.2018.
//

#ifndef CYCLIC_BUFFER_H
#define CYCLIC_BUFFER_H


#include <vector>

class cyclic_buffer {
    int max_size, position, end, PSIZE;
    char *array;
public:
    cyclic_buffer(int PSIZE) : max_size(2 * PSIZE), position(0), end(0), PSIZE(PSIZE) { array = new char[max_size]; }

    int size();

    void push(char *a, int n);

    std::vector<char> pop();

    ~cyclic_buffer() {
        delete[] array;
    }
};


#endif //ZADANIE2_SIK_CYCLIC_BUFFER_H

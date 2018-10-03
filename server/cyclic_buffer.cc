//
// Created by mkfil on 08.06.2018.
//

#include <cstring>
#include <cstdio>
#include "cyclic_buffer.h"

int cyclic_buffer::size() {
    return (position - end + max_size) % max_size;
}

void cyclic_buffer::push(char *a, int n) {
    int f = std::min(n, max_size - position);
    int s = n - f;
    memcpy(array + position, a, f);
    position += f;
    if (position == max_size) position = 0;
    if (s > 0) {
        memcpy(array, a + f, s);
        position = s;
    }

}

std::vector<char> cyclic_buffer::pop() {
    char a[PSIZE];
    int i = std::min(PSIZE, max_size - end);
    memcpy(a, array + end, i);
    end += i;
    if (end == max_size) end = 0;
    if (i != PSIZE) {
        memcpy(a + i, array, PSIZE - i);
        end = PSIZE - i;
    }
    std::vector<char> v;
    v.assign(a, a + PSIZE);
    return v;
}

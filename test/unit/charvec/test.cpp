//
// Created by lukemartinlogan on 12/5/21.
//
#include <cstdio>
#include <cstdlib>
#include <cstring>

struct foo {
    int a;
    char data[];
};

int main() {
    printf("%lu %lu\n", sizeof(struct foo), sizeof(int));

    struct foo *hi = (struct foo*)malloc(sizeof(200));
    hi->a = 42;
    strcpy(hi->data, "3234523");
    printf("%s\n", hi->data);
}
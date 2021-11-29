//
// Created by lukemartinlogan on 11/29/21.
//

#include <unordered_map>
#include <labstor/types/basics.h>


int main(){
    labstor::id name("Dummy");
    labstor::id name2("Dummy297428947284ry");
    name2.key[5] = 0;

    printf("%s == %s: %d\n", name.key, name2.key, name == name2);
    printf("%lu %lu\n", std::hash<labstor::id>()(name), std::hash<labstor::id>()(name2));
}
//
// Created by lukemartinlogan on 12/4/21.
//

#include <labstor/userspace/util/path_parser.h>
#include <iostream>

int main() {
    std::string path = "/home/${HOME}/hi/${HOME}/${HOME2}/.txt";
    std::cout << scs::path_parser(path) << std::endl;
}
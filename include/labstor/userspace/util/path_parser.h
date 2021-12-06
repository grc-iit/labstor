//
// Created by lukemartinlogan on 12/4/21.
//

#ifndef LABSTOR_PATH_PARSER_H
#define LABSTOR_PATH_PARSER_H

#include <cstdlib>
#include <string>
#include <regex>
#include <list>

namespace scs {

std::string path_parser(std::string path) {
    std::smatch env_names;
    std::regex expr("\\$\\{[^\\}]+\\}");
    if(!std::regex_search(path, env_names, expr)) {
        return path;
    }
    for(auto &env_name_re : env_names) {
        std::string to_replace = std::string(env_name_re);
        std::string env_name = to_replace.substr(2, to_replace.size()-3);
        std::string env_val = env_name;
        try {
            env_val = getenv(env_name.c_str());
        } catch(...) {
        }
        std::regex replace_expr("\\$\\{" + env_name + "\\}");
        path = std::regex_replace(path, replace_expr, env_val);
    }
    return path;
}

}

#endif //LABSTOR_PATH_PARSER_H

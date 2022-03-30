//
// Created by lukemartinlogan on 3/29/22.
//

#include <unordered_map>
#include <labstor/userspace/util/timer.h>

int main(int argc, char **argv) {
    std::unordered_map<std::string, int> map1;
    std::unordered_map<int,int> map2;

    //File create
    labstor::HighResMonotonicTimer t;
    t.Resume();
    for(int i = 0; i < 100000; ++i) {
        map1.emplace(std::string("hello") + std::to_string(i), i);
        map2.emplace(i,i+1);
        for(int j = 0; j<3; ++j) { malloc(1); }
    }
    t.Pause();

    printf("ops/sec: %lf\n", 100000/t.GetSec());
}
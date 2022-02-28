//
// Created by lukemartinlogan on 2/17/22.
//

#include <unordered_map>
#include <labstor/userspace/util/timer.h>

int main() {
    int ops = 1024;
    labstor::HighResCpuTimer t;
    std::unordered_map<int,int> map;
    for(int i = 0; i < 10; ++i) {
        map.emplace(i, i);
    }

    t.Resume();
    for(int i = 0; i < 1024; ++i) {
        int k = map[0];
        k += 1;
    }
    t.Pause();

    printf("OPS PER SEC: %d\n", ops/t.GetUsec());
}
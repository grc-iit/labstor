//
// Created by lukemartinlogan on 11/25/21.
//

#include <labstor/types/data_structures/shmem_int_map.h>
#include <cstdio>

int main() {
    uint32_t region_size = 1024;
    void *region = malloc(region_size);
    labstor::ipc::int_map<uint32_t, uint32_t> map;
    map.Init(region, region_size, 8);
    uint32_t value;
    int num_inserts = map.GetNumBuckets() + map.GetOverflow();

    //Test find
    for(int i = 0; i < num_inserts; ++i) {
        LABSTOR_ERROR_HANDLE_START()
        value = map[i];
        if(value != i + 1) {
            printf("Value not set properly\n");
            exit(1);
        }
        LABSTOR_ERROR_HANDLE_END()
    }
    printf("Finished finding\n");

    //Test deletion
    for(int i = 0; i < num_inserts; ++i) {
        map.Remove(i);
    }
    printf("Finished removing\n");

    //Retest find
    for(int i = 0; i < num_inserts; ++i) {
        if(map.Find(i, value)) {
            printf("Was still able to find the value %d\n", i);
            exit(1);
        }
    }
    printf("Finished refinding\n");
}
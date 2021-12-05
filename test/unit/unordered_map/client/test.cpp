//
// Created by lukemartinlogan on 11/25/21.
//

#include <labstor/userspace/util/errors.h>
#include <labstor/types/data_structures/shmem_string_map.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>

int main() {
    uint32_t region_size = 8192;
    uint32_t map_region_size = 2048;
    void *region = malloc(region_size);
    char *string_region = (char*)region;
    char *map_region = (char*)region + region_size - map_region_size;
    labstor::ipc::string_map map;
    map.Init(region, map_region, map_region_size, 8);
    uint32_t value;
    //int num_inserts = map.GetNumBuckets() + map.GetOverflow();
    int num_inserts = 50;
    printf("Num inserts/overflow: %d\n", num_inserts);

    //Test set
    for(int i = 0; i < num_inserts; ++i) {
        LABSTOR_ERROR_HANDLE_START()
        labstor::ipc::string str;
        str.Init(string_region + (i+16)*16, "hi" + std::to_string(i));
        if(!map.Set(str, i+1)) {
            printf("Failed: %d\n", i);
            exit(1);
        }
        LABSTOR_ERROR_HANDLE_END()
    }
    printf("Finished setting\n");

    //Test find
    for(int i = 0; i < num_inserts; ++i) {
        LABSTOR_ERROR_HANDLE_START()
        labstor::ipc::string str;
        str.Attach(string_region + (i+16)*16);
        value = map[str];
        if(value != i + 1) {
            printf("Value not set properly: %d\n", i);
            exit(1);
        }
        LABSTOR_ERROR_HANDLE_END()
    }
    printf("Finished finding\n");

    //Test deletion
    for(int i = 0; i < num_inserts; ++i) {
        labstor::ipc::string str;
        str.Attach(string_region + (i+16)*16);
        map.Remove(str);
    }
    printf("Finished removing\n");

    //Retest find
    for(int i = 0; i < num_inserts; ++i) {
        labstor::ipc::string str;
        str.Attach(string_region + (i+16)*16);
        if(map.Find(str, value)) {
            printf("Was still able to find the value %d\n", i);
            exit(1);
        }
    }
    printf("Finished refinding\n");
}
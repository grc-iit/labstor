
/*
 * Copyright (C) 2022  SCS Lab <scslab@iit.edu>,
 * Luke Logan <llogan@hawk.iit.edu>,
 * Jaime Cernuda Garcia <jcernudagarcia@hawk.iit.edu>
 * Jay Lofstead <gflofst@sandia.gov>,
 * Anthony Kougkas <akougkas@iit.edu>,
 * Xian-He Sun <sun@iit.edu>
 *
 * This file is part of LabStor
 *
 * LabStor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <labstor/userspace/util/errors.h>
#include <labstor/types/data_structures/unordered_map/shmem_string_map.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>

int main() {
    uint32_t region_size = 8192;
    int num_inserts = 50;
    int max_collisions = 16;
    int num_buckets = 2*num_inserts + max_collisions;
    uint32_t map_region_size = labstor::ipc::mpmc::string_map::GetSize(num_buckets);
    void *region = malloc(region_size);
    char *string_region = (char*)region;
    char *map_region = (char*)region + region_size - map_region_size;
    labstor::ipc::mpmc::string_map map;
    uint32_t value;
    LABSTOR_ERROR_HANDLE_START()
    map.Init(region, map_region, map_region_size, max_collisions);
    printf("Num inserts: %d %d\n", num_inserts, map.GetNumBuckets());
    LABSTOR_ERROR_HANDLE_END()

    //Test set
    for(int i = 0; i < num_inserts; ++i) {
        LABSTOR_ERROR_HANDLE_START()
        labstor::ipc::string str;
        str.Init(string_region + (i+16)*16, "hi" + std::to_string(i));
        if(!map.Set(str, i)) {
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
        if(value != i) {
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
        if(!map.Remove(str)) {
            printf("Error removing %d\n", i);
            exit(1);
        }
    }
    printf("Finished removing\n");

    //Retest find
    for(int i = 0; i < num_inserts; ++i) {
        labstor::ipc::string str;
        str.Attach(string_region + (i+16)*16);
        if(map.Find(str, value)) {
            printf("Was still able to find map[%d]\n", i);
            exit(1);
        }
    }
    printf("Finished refinding\n");
}
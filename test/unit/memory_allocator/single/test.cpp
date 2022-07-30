
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

#include <vector>
#include <labstor/types/allocator/shmem_allocator.h>
#include <labstor/types/allocator/private_shmem_allocator.h>

uint32_t page_size = 128;
uint32_t num_pages = 64;
size_t region_size = page_size * num_pages;
void *region;

labstor::GenericAllocator* private_allocator_test() {
    labstor::ipc::private_shmem_allocator *allocator = new labstor::ipc::private_shmem_allocator();
    allocator->Init(region, region, region_size, page_size);
    return allocator;
}

labstor::GenericAllocator* multicore_allocator_test() {
    labstor::ipc::shmem_allocator *allocator = new labstor::ipc::shmem_allocator();
    allocator->Init(region, region, region_size, page_size, 4);
    return allocator;
}

void single_allocate_test(labstor::GenericAllocator *allocator) {
    void *page;
    int *intpg, i = 0;
    std::vector<void*> pages;

    //Allocate requests
    while(page = allocator->Alloc(page_size, 0)) {
        intpg = (int*)page;
        *intpg = i++;
        pages.emplace_back(page);
        printf("PAGE: %d\n", (uint32_t)((size_t)page - (size_t)region));
    }
    printf("\n");

    //View request values
    for(int i = 0; i < pages.size(); ++i) {
        int *page = (int*)pages[i];
        printf("PAGE VALUE: %d\n", *page);
    }
    printf("\n");

    //Free requests
    for(uint32_t i = 0; i < pages.size(); ++i) {
        allocator->Free(pages[i]);
    }

    //Allocate requests
    for(uint32_t i = 0; i < pages.size(); ++i) {
        page = allocator->Alloc(page_size, 0);
        if(page == nullptr) {
            printf("Couldn't reallocate all pages\n");
            exit(1);
        }
        intpg = (int*)page;
        printf("PAGE: %d\n", (uint32_t)((size_t)page - (size_t)region));
        *intpg = 5;
    }
    printf("\n");
}

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("USAGE: ./test [allocator_type]\n");
        exit(1);
    }
    std::string allocator_type = argv[1];
    region = malloc(region_size);
    if(allocator_type == "PRIVATE") {
        single_allocate_test(private_allocator_test());
    }
    if(allocator_type == "MULTICORE") {
        single_allocate_test(multicore_allocator_test());
    }
}
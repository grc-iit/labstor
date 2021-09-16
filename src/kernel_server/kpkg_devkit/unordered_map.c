//
// Created by lukemartinlogan on 9/14/21.
//

#include "types.h"
#include "unordered_map.h"

#include <linux/vmalloc.h>

inline int hash(struct labstor_id id) {
    int hash = 0;
    int i;
    for(i = 0; i < 255; ++i) {
        hash += id.key[i] * id.key[(i+1)];
    }
    return hash;
}

void unordered_map_init(struct unordered_map *map, int nbuckets) {
    map->nbuckets_ = nbuckets;
    map->buckets_ = vzalloc(nbuckets*sizeof(void*));
}

void unordered_map_free(struct unordered_map *map) {
    vfree(map->buckets_);
}

int unordered_map_add(struct unordered_map *map, struct labstor_id id, void *data) {
    int i;
    int idx = hash(id) % map->nbuckets_;
    for(i = 0; i < 256; ++i) {
        if(map->buckets_[idx + i] == 0) {
            map->buckets_[idx + i] = data;
            return idx + i;
        }
    }
    return -1;
}

void unordered_map_remove_idx(struct unordered_map *map, int idx) {
    map->buckets_[idx] = 0;
}

void* unordered_map_get_idx(struct unordered_map *map, int idx) {
    return map->buckets_[idx];
}
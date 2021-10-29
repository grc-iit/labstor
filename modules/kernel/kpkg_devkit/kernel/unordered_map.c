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
        if(id.key[i] == 0) { break; }
        hash += id.key[i] * id.key[(i+1)];
    }
    return hash;
}

void unordered_map_init(struct unordered_map *map, int nbuckets) {
    map->nbuckets_ = nbuckets;
    map->buckets_ = vzalloc(nbuckets*sizeof(struct bucket));
}
EXPORT_SYMBOL(unordered_map_init);


void unordered_map_free(struct unordered_map *map) {
    vfree(map->buckets_);
}
EXPORT_SYMBOL(unordered_map_free);

int unordered_map_add(struct unordered_map *map, struct labstor_id id, void *data) {
    int i;
    int idx;
    int cur_idx;
    idx = hash(id) % map->nbuckets_;
    for(i = 0; i < map->nbuckets_; ++i) {
        cur_idx = (idx + i)%map->nbuckets_;
        if(map->buckets_[cur_idx].id.key[0] == 0) {
            map->buckets_[cur_idx].id = id;
            map->buckets_[cur_idx].data = data;
            return cur_idx;
        }
    }
    return -1;
}
EXPORT_SYMBOL(unordered_map_add);

void* unordered_map_get(struct unordered_map *map, struct labstor_id id, int *runtime_id) {
    int i;
    int idx;
    int cur_idx;
    idx = hash(id) % map->nbuckets_;
    for(i = 0; i < map->nbuckets_; ++i) {
        cur_idx = (idx + i)%map->nbuckets_;
        if(strcmp(map->buckets_[cur_idx].id.key, id.key) == 0) {
            *runtime_id = cur_idx;
            return map->buckets_[cur_idx].data;
        }
    }
    return NULL;
}
EXPORT_SYMBOL(unordered_map_get);

void* unordered_map_get_idx(struct unordered_map *map, int idx) {
    return map->buckets_[idx].data;
}
EXPORT_SYMBOL(unordered_map_get_idx);

void unordered_map_remove(struct unordered_map *map, struct labstor_id id) {
    int i;
    int idx;
    int cur_idx;
    idx = hash(id) % map->nbuckets_;
    for(i = 0; i < map->nbuckets_; ++i) {
        cur_idx = (idx + i)%map->nbuckets_;
        if(strcmp(map->buckets_[cur_idx].id.key, id.key) == 0) {
            memset(&map->buckets_[idx].id, 0, sizeof(struct labstor_id));
            return;
        }
    }
}
EXPORT_SYMBOL(unordered_map_remove);

void unordered_map_remove_idx(struct unordered_map *map, int idx) {
    memset(&map->buckets_[idx].id, 0, sizeof(struct labstor_id));
}
EXPORT_SYMBOL(unordered_map_remove_idx);

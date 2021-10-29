//
// Created by lukemartinlogan on 9/13/21.
//

#ifndef LABSTOR_KERNEL_SERVER_UNORDERED_MAP
#define LABSTOR_KERNEL_SERVER_UNORDERED_MAP

#include <labstor/types/basics.h>
#include <linux/types.h>

struct bucket {
    struct labstor_id id;
    void *data;
};

struct unordered_map {
    int nbuckets_;
    struct bucket *buckets_;
};

void unordered_map_init(struct unordered_map *map, int nbuckets);
void unordered_map_free(struct unordered_map *map);
int unordered_map_add(struct unordered_map *map, struct labstor_id id, void *data);
void unordered_map_remove(struct unordered_map *map, struct labstor_id id);
void unordered_map_remove_idx(struct unordered_map *map, int idx);
void* unordered_map_get_idx(struct unordered_map *map, int idx);
void* unordered_map_get(struct unordered_map *map, struct labstor_id id, int *runtime_id);

#endif //LABSTOR_KERNEL_SERVER_UNORDERED_MAP

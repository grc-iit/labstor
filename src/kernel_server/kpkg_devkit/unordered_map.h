//
// Created by lukemartinlogan on 9/13/21.
//

#ifndef LABSTOR_KERNEL_SERVER_UNORDERED_MAP
#define LABSTOR_KERNEL_SERVER_UNORDERED_MAP

struct unordered_map {
    int nbuckets_;
    void **buckets_;
};

void unordered_map_init(struct unordered_map *map, int nbuckets);
void unordered_map_free(struct unordered_map *map);
int unordered_map_add(struct unordered_map *map, struct labstor_id id, void *data);
void unordered_map_remove_idx(struct unordered_map *map, int idx);
void* unordered_map_get_idx(struct unordered_map *map, int idx);

#endif //LABSTOR_KERNEL_SERVER_UNORDERED_MAP

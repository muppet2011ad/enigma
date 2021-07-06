#ifndef hashmap_h

#define hashmap_h

#include <stdlib.h>
#include "linked_list.h"

struct kv_pair_structure {
    void *key;
    size_t key_length;
    void *val;
};

typedef struct kv_pair_structure *kv_pair;

struct hashmap_structure {
    linked_list *buckets;
    int num_buckets;
    int buckets_used;
    int nmeb;
    int (*key_eq_func)(void*, void*);
    int (*hash_function)(void*, int);
    double max_load_factor;
    short resize_in_progress;
};

typedef struct hashmap_structure *hashmap;

hashmap new_hashmap(int num_buckets, double max_load_factor, int (*key_equality_func)(void*, void*), int (*hash_function)(void*, int));
int add_to_hashmap(hashmap h, void *key, size_t key_length, void *value);
int is_key_in_hashmap(hashmap h, void *key, int key_length);
void *get_value_from_hashmap(hashmap h, void *key, int key_length);
void remove_from_hashmap(hashmap h, void *key, int key_length);
kv_pair *hashmap_to_array (hashmap h);
kv_pair create_virtual_pair(void *key);
void destroy_hashmap(hashmap h);

#endif
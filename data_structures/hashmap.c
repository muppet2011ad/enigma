#include "hashmap.h"

hashmap new_hashmap(int num_buckets, double max_load_factor, int (*key_equality_func)(void*, void*), int (*hash_function)(void*, int)) {
    hashmap h = malloc(sizeof(struct hashmap_structure));
    h->buckets = calloc(num_buckets, sizeof(linked_list*));
    h->num_buckets = num_buckets;
    h->key_eq_func = key_equality_func;
    h->hash_function = hash_function;
    h->buckets_used = 0;
    h->nmeb = 0;
    h->max_load_factor = max_load_factor;
    h->resize_in_progress = 0;
    return h;
}

void resize_hashmap(hashmap h) {
    h->resize_in_progress = 1;
    linked_list *new_buckets = calloc(2*h->num_buckets, sizeof(linked_list*));
    linked_list *old_buckets = h->buckets;
    kv_pair *members = hashmap_to_array(h);
    h->buckets = new_buckets;
    int old_num_buckets = h->num_buckets;
    h->num_buckets = 2*h->num_buckets;
    h->buckets_used = 0;
    int nmeb = h->nmeb;
    h->nmeb = 0;
    for (int i = 0; i < nmeb; i++) {
        kv_pair pair = members[i];
        add_to_hashmap(h, pair->key, pair->key_length, pair->val);
    }
    for (int i = 0; i < old_num_buckets; i++) {
        if (old_buckets[i]) {
            destroy_linked_list(old_buckets[i]);
        }
    }
    free(old_buckets);
    free(members);
    h->resize_in_progress = 0;
}

int add_to_hashmap(hashmap h, void *key, size_t key_length, void *value) {
    int hash = h->hash_function(key, key_length) % h->num_buckets;
    kv_pair pair = malloc(sizeof(struct kv_pair_structure));
    pair->key = key;
    pair->key_length = key_length;
    pair->val = value;
    if (!h->buckets[hash]) {
        h->buckets[hash] = new_linked_list();
        h->buckets_used++;
    }
    kv_pair key_pair = create_virtual_pair(key);
    if (find_in_linked_list(h->buckets[hash], key_pair, h->key_eq_func)) {
        free(key_pair);
        free(pair);
        return 0;
    }
    free(key_pair);
    add_to_linked_list(h->buckets[hash], pair);
    h->nmeb++;
    if (!h->resize_in_progress && (double) h->nmeb/(double) h->num_buckets > h->max_load_factor) {
        resize_hashmap(h);
    }
    return 1;
}

int is_key_in_hashmap(hashmap h, void *key, size_t key_length) {
    int hash = h->hash_function(key, key_length) % h->num_buckets;
    if (h->buckets[hash]) {
        kv_pair key_pair = create_virtual_pair(key);
        ll_node n = find_in_linked_list(h->buckets[hash], key_pair, h->key_eq_func);
        free(key_pair);
        return n != NULL;
    }
    else {
        return 0;
    }
}

void *get_value_from_hashmap(hashmap h, void *key, size_t key_length) {
    int hash = h->hash_function(key, key_length) % h->num_buckets;
    if (h->buckets[hash]) {
        kv_pair key_pair = create_virtual_pair(key);
        ll_node n = find_in_linked_list(h->buckets[hash], key_pair, h->key_eq_func);
        free(key_pair);
        if (n) {
            return ((kv_pair) n->data)->val;
        }
        else {
            return NULL;
        }
    }
    return NULL;
}

void remove_from_hashmap(hashmap h, void *key, size_t key_length) {
    int hash = h->hash_function(key, key_length) % h->num_buckets;
    if (h->buckets[hash]) {
        kv_pair key_pair = create_virtual_pair(key);
        remove_data_from_linked_list(h->buckets[hash], key_pair, h->key_eq_func);
        h->nmeb--;
        free(key_pair);
    }
}

kv_pair *hashmap_to_array (hashmap h) {
    kv_pair *out;
    out = calloc(h->nmeb, sizeof(struct kv_pair_structure));
    int x = 0;
    for (int i = 0; i < h->num_buckets; i++) {
        if (h->buckets[i]) {
            kv_pair *kv_list = (kv_pair*) linked_list_to_array(h->buckets[i]);
            for (int j = 0; j < h->buckets[i]->length; j++) {
                out[x] = kv_list[j];
                x++;
            }
            free(kv_list);
        }
    }
    return out;
}

kv_pair create_virtual_pair(void *key) {
    kv_pair p = malloc(sizeof(struct kv_pair_structure));
    p->key = key;
    p->val = NULL;
    return p;
}

void destroy_hashmap(hashmap h, short destroy_data) {
    if (destroy_data) {
        kv_pair *members = hashmap_to_array(h);
        for (int i = 0; i < h->nmeb; i++) {
            free(members[i]->key);
            free(members[i]->val);
        }
        free(members);
    }
    for (int i = 0; i < h->num_buckets; i++) {
        if (h->buckets[i]) {
            destroy_linked_list(h->buckets[i]);
        }
    }
    free(h->buckets);
    free(h);
}
//TODO: Linked list and hashmap

#include "linked_list.h"

linked_list new_linked_list() {
    linked_list l = malloc(sizeof(struct linked_list_structure));
    l->first = NULL;
    l->last = NULL;
    l->length = 0;
    return l;
}

void add_to_linked_list(linked_list l, void *data) {
    ll_node n = malloc(sizeof(struct node_structure));
    n->data = data;
    n->next = NULL;
    n->prev = l->last;
    if (l->length == 0) {
        l->first = n;
    }
    if (l && l->last) {
        l->last->next = n;
    }
    l->last = n;
    l->length += 1;
}

int add_at_index_to_linked_list(linked_list l, void *data, int index) {
    ll_node new = malloc(sizeof(struct node_structure));
    new->data = data;
    if (index == 0) {
        new->next = l->first;
        new->prev = NULL;
        l->first = new;
        if (l->length == 0) {
            l->last = new;
        }
        l->length += 1;
        return 1;
    }
    else{
        if (index == l->length) {
            add_to_linked_list(l, data);
            return 1;
        }
        ll_node aft = find_at_index_in_linked_list(l, index);
        if (aft) {
            ll_node bef = aft->prev;
            new->next = aft;
            new->prev = bef;
            bef->next = new;
            aft->prev = new;
            l->length += 1;
        }
        else {
            destroy_node(new);
            return 0;
        }
    }
}

ll_node find_in_linked_list(linked_list l, void *search_key, int (*eq_func)(void*, void*)) {
    ll_node next = l->first;
    while (next != NULL) {
        if (eq_func(next->data, search_key) == 1) {
            return next;
        }
        next = next->next;
    }
    return NULL;
}

int get_index_in_linked_list(linked_list l, void *search_key, int (*eq_func)(void*, void*)) {
    ll_node next = l->first;
    int counter = 0;
    while (next != NULL) {
        if (eq_func(next->data, search_key) == 1) {
            return counter;
        }
        next = next->next;
        counter++;
    }
    return -1;
}

ll_node find_at_index_in_linked_list(linked_list l, int index) {
    ll_node next = l->first;
    for (int i = 0; i < index; i++) {
        if (i >= l->length) {
            return next;
        }
        next = next->next;
    }
    return next;
}

void remove_from_linked_list(linked_list l, ll_node n) {
    if (n->prev != NULL){
        n->prev->next = n->next;
    }
    if (n->next != NULL){
        n->next->prev = n->prev;
    }
    l->length -= 1;
    if (n == l->first) {
        l->first = n->next;
    }
    if (n == l->last) {
        l->last = n->prev;
    }
    destroy_node(n);
}

void remove_data_from_linked_list(linked_list l, void *data, int (*eq_func)(void*, void*)) {
    ll_node target = find_in_linked_list(l, data, eq_func);
    if (target) {
        remove_from_linked_list(l, target);
    }
}

void remove_index_from_linked_list(linked_list l, int index) {
    ll_node result = find_at_index_in_linked_list(l, index);
    if (result) {
        remove_from_linked_list(l, result);
    }
}

void **linked_list_to_array(linked_list l) {
    void **out = calloc(l->length, sizeof(void*));
    ll_node n = l->first;
    for (int i = 0; i < l->length; i++) {
        out[i] = n->data;
        n = n->next;
    }
    return out;
}

void destroy_linked_list(linked_list l) {
    ll_node n = l->first;
    for (int i = 0; i < l->length; i++) {
        ll_node next = n->next;
        destroy_node(n);
        n = next;
    }
    free(l);
}

void destroy_node(ll_node n) { // NOTE: Assumes the node has already been removed from the list - calling this on a node still in the list will lead to a segfault later on
    free(n->data);
    free(n);
}
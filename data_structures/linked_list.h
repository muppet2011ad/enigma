#include <stdlib.h>

typedef struct node_structure *ll_node;

struct node_structure {
    void *data;
    ll_node next;
    ll_node prev;
};

struct linked_list_structure {
    ll_node first;
    ll_node last;
    int length;
};

typedef struct linked_list_structure *linked_list;

linked_list new_linked_list();
void add_to_linked_list(linked_list l, void *data);
int add_at_index_to_linked_list(linked_list l, void *data, int index);
ll_node find_in_linked_list(linked_list l, void *search_key, int (*eq_func)(void*, void*));
int get_index_in_linked_list(linked_list l, void *search_key, int (*eq_func)(void*, void*));
ll_node find_at_index_in_linked_list(linked_list l, int index);
void remove_from_linked_list(linked_list l, ll_node n);
void remove_data_from_linked_list(linked_list l, void *data, int (*eq_func)(void*, void*));
void remove_index_from_linked_list(linked_list l, int index);
void **linked_list_to_array(linked_list l);
void destroy_linked_list(linked_list l);
void destroy_node(ll_node n);
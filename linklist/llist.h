#ifndef __LLIST_H__
#ifdef __cplusplus
extern "C"{
#endif


#include <stdbool.h>

typedef struct slist_node{
    struct slist_node* next;
    void* item;
}slnode;


slnode* slist_new( void* item );
slnode* slist_append( void* item );
slnode* slist_prepend( void* item );
slnode* slist_delete( void* item );
slnode* slist_search( void* item );
slnode* slist_traverse_n( int n );



slnode* slist_insert_before( slnode* node );
slnode* slist_insert_after( slnode* target, );
slnode* slist_insert_static( slnode*  void* item );



#ifdef __cplusplus
}
#endif


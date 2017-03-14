/* 
 * hashtable.h - header file for hashtable data structure
 *  
 *
 * Samuel Ching, April 2016
 * Building off Prof Kotz's tree.h file
 */

#ifndef __HASHTABLE_H
#define __HASHTABLE_H

/**************** global types ****************/
typedef struct hashtable hashtable_t; // opaque to users of the module

/**************** functions ****************/

/*create a new empty hash table data structure.*/
hashtable_t *hashtable_new(const int num_slots, void (*itemdelete)(void *data));

/*return data for the given key, or NULL if not found.*/
void *hashtable_find(hashtable_t *ht, char *key); 

/*return false if key already exists, and true if new item was inserted.*/
bool hashtable_insert(hashtable_t *ht, char *key, void *data); 

/** Deletes the entire hashtable and its contents **/
void hash_delete(hashtable_t* ht);

/* Iterate over all items in hashtable (in undefined order)*/
void hashtable_iterate(hashtable_t *ht, void (*itemfunc)(void *arg, char *key, void *data), void *arg);

/* delete function for codedrop hashtable */
void code_delete(void* data);

/* delete function for team hashtable */
void team_delete(void* data);

/* delete function for agent hashtable*/
void agent_delete(void* data);

#endif // __HASHTABLE_H
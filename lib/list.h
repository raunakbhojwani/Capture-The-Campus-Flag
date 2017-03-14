/* 
 * list.h - header file for list data structure
 *  
 *
 * Samuel Ching, April 2016
 * Building off Prof Kotz's tree.h file
 */

#ifndef __LIST_H
#define __LIST_H

/**************** global types ****************/
typedef struct list list_t; // opaque to users of the module

/**************** functions ****************/

/* Create a new (empty) list */
list_t* list_new( void (*itemdelete)(void *data));

/* Searches the list, returns data for the given key, or NULL if key is not found. */
void *list_find(list_t *list, char *key); 

/* Inserts the key (with its corresponding data into the list) and returns true. Else if key is already in the list, returns false.*/
bool list_insert(list_t *list, char *key, void *data); 

/* Deletes the whole list. */
void list_delete(list_t* list);

/* Iterate over all items in list (in undefined order)*/
void list_iterate(list_t *list, void (*itemfunc)(void *arg, char *key, void *data), void *arg);

/* Deletes a Specific Node in the List */
void listnode_delete(list_t* list, char* key);



#endif // __list_h

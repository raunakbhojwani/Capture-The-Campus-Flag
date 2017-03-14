/* 
 * hashtable.c - hashtable data structure implemented using list.c
 *  
 * Samuel Ching, April 2016
 * Based on Prof Kotz's CS50 demo - names.c & tree.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "list.h"
#include "hashtable.h"
#include "jhash.h"
#include "team.h"
#include "codedrop.h"
#include "agent.h"


/**************** global variables ****************/

int CALLOC_INT_HASH = 1;

/**************** local types ****************/

/**************** global types ****************/
typedef struct hashtable{
	int size; // size of hashtable
	list_t** table; // storage table itself
	void (*itemdelete)(void *data);
  	void (*itemfunc)(void *arg, char *key, void *data); //destructor function which deletes the data stored in the bag
} hashtable_t;

/**************** global functions ****************/
/* that is, visible outside this file */
/* see hashtable.h for comments about exported functions */

/**************** local functions ****************/
/* not visible outside this file */


/** hashtable_new() initializes a new hashtable
* It takes the number of slots as its parameter
* If the hashtable is incorrectly created or the number of slots which the user inputs is less than 1, a NULL value is returned
* If the table structure was incorrectly initialized, return NULL.
* If everything is initialized well, return the hashtable itself. 
*/

hashtable_t *hashtable_new(const int num_slots, void (*itemdelete)(void *data)){

	if (num_slots<1)
		return NULL;

	hashtable_t* hashtable = calloc(CALLOC_INT_HASH, sizeof(hashtable_t)); // create memory for the hashtable data structure

	if (hashtable == NULL){
		free(hashtable);
		return NULL;
	}
	else{
		hashtable->table = calloc(num_slots, sizeof(list_t*));
		if (hashtable->table == NULL){ 
			free(hashtable);		// if the array with list as slots was not initialized properly
			return NULL;
		}
		else{
			hashtable->size = num_slots; // initialize the value of the size of the hashtable to the number of slots.
			hashtable->itemdelete = itemdelete;
			return hashtable;
		}
	}
}

/** hashtable_find() takes a key and looks up the hashtable to check if its inside the hashtable 
* It takes the the hashtable and the key as parameters
* If the hashtable is non-existent, return NULL
* If the hashtable exists, it calls list_find function in list() to check if the key is in the list
* If the key is in the list, the list_find function will return the data associated with the key.
* Else, if the key is not in the list, it will return NULL.
*/

void *hashtable_find(hashtable_t *ht, char *key){
	 
	 // check if that the hashtable is not null
	 if (ht == NULL){
	 	return NULL;
	 }

	 else{
	 	int hashkey = JenkinsHash(key,ht->size); // find the hashkey using the hashfunction, modulo takes on the number of slots in the hashtable.
	 	list_t* list = ht->table[hashkey];
	 	return list_find(list,key); // call list_find to find the key in the list
	 }
}

/** hashtable_insert() takes a key, along with its corresponding data, and inserts it into the hashtable if the key has not been used before.
* It takes the hashtable, the key, along with the data as parameters
* If the hashtable is non-existent, return NULL.
* If the hashtable exists and if the key exists in the hashtable, return 'false'.
* If the hashtable exists and if the key does not exist, check the array slot to see if there is an existing list in the slot
* If there is, then use the list module's list_insert function to insert the key and data into the list. list_insert returns True if successful.
* Else, create a new list and insert the key and data into the list. list_insert returns True if successful.
*/ 

bool hashtable_insert(hashtable_t *ht, char *key, void *data){

	// check if that the hashtable is not null

	if (ht == NULL){
		return NULL;
	}
	else{
		
		// checks to see if there is already an existing key.
		// If there is, return NULL. 
		// Else, create a new key

		if (hashtable_find(ht,key) == NULL){
			int hashkey = JenkinsHash(key,ht->size); // uses the hash function to find the hashkey, modulo takes on the number of slots in the hashtable.
			list_t* list = ht->table[hashkey]; // indexes into the array slot using the hashkey
			if (list == NULL){
				ht->table[hashkey] = list_new(ht->itemdelete); // initialize the new list with listDelete destructor function passed in 
				return list_insert(ht->table[hashkey],key,data);
			}
			else{
				return list_insert(list,key,data); // call list insert function if the list was found
			}
		}
		else{
			return false;
		}
	}

}

void hash_delete(hashtable_t* ht)
{
	if (ht == NULL){
		printf("Hashtable is null\n\n");
		return; // if the hashtable is non-existent, print an error message and allow it to silently fall
	}
	else if (ht->table == NULL){
		printf("Hashtable structure is null\n\n");
		return; // if the hashtable structure is non-existent, print an error message and allow it to silently fall
	} else {
		for (int i = 0; i < ht->size; i++){
			if (ht->table[i] != NULL){
				list_delete(ht->table[i]);
			}
		}
		free(ht->table);
		free(ht);
	}
}

/* Iterate over all items in hashtable (in undefined order):
 * call itemfunc for each item, with (arg, key, data).
 */
void hashtable_iterate(hashtable_t *ht, void (*itemfunc)(void *arg, char *key, void *data), void *arg)
{
	if (ht == NULL){
		return; // if the hashtable is non-existent, print an error message and allow it to silently fall
	}
	else if (ht->table == NULL){
		return; // if the hashtable structure is non-existent, print an error message and allow it to silently fall
	} else {
		for (int i = 0; i < ht->size; i++){
			if (ht->table[i] != NULL){
				list_iterate(ht->table[i], itemfunc, arg);
			}
		}
	}
}



/*** Gen Delete Functions **/

/** team_delete()
destructor function which will be passed in to the team hashtable*/

void team_delete(void* data)
{
	if (data != NULL){
		team_del(data);
	}
}


/** code_delete()
destructor function which will be passed in to the code hashtable*/

void code_delete(void* data)
{
	if (data != NULL){
		codedrop_del(data);
	}
}

/** agent_delete() 
destructor function which will be passed into the agent hashtable */
void agent_delete(void* data)
{
	if (data != NULL){
		agent_del(data);
	}
}




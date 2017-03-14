/* 
 * list.c - simple list module implemented using Linked Lists.
 *  
 * Samuel Ching, April 2016
 * Based on Prof Kotz's CS50 demo - names.c & tree.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "list.h" 

/**************** global variables ****************/

int CALLOC_INT_LIST = 1; // multiple of each type when calling calloc

/**************** local types ****************/
typedef struct listnode {
  void* data;
  char* key;               
  struct listnode *next;  
} listnode_t;

/**************** global types ****************/
typedef struct list {
  struct listnode *head;                     // head of the list
  void (*itemdelete)(void *data); // function that prints a node - pointer to a function which takes those arguments 
  void (*itemfunc)(void *arg, char *key, void *data); //destructor function which deletes the data stored in the bag
} list_t;

/**************** global functions ****************/
/* that is, visible outside this file */
/* see list.h for comments about exported functions */

/**************** local functions ****************/
/* not visible outside this file */

/* Creates a new node for the list*/
listnode_t* newNode(char* key, void* data);

/* Frees a single node in the list*/
void freeNode(listnode_t* listnode);

/* Helper function to delete all the nodes in a list and free the list itself*/ 
void list_delete_helper(list_t* list, listnode_t* listnode);

/** list_new() creates a new list and allocates the memory for the new list 
* Takes in a 'destructor' function which frees the memory allocated to the list->data field.
* If the list is incorrectly allocated, the destructor function is NULL or the head of the list is incorrectly allocated, return NULL.
* Else, return the newly created list.
*/

list_t* list_new( void (*itemdelete)(void *data)) 
{
  list_t* list = calloc(CALLOC_INT_LIST, sizeof(list_t));

  if (list == NULL) {
    return NULL; // error allocating list
  } else {

    // initialize contents of list structure
    list->head = calloc(CALLOC_INT_LIST, sizeof(listnode_t));

    if (list->head == NULL){
      return NULL;
    }
    if (itemdelete == NULL){
      return NULL;
    }

    list->itemdelete = itemdelete;
    return list; 
  }
}

/** list_find() searches the given list for the specified key
* It takes in a list and a key as params
* If the given list is NULL or the key is not found in the given list, it returns NULL
* If the key is found, the data stored with the key is returned. 
*/

void *list_find(list_t *list, char *key) 
{
  if (list == NULL){
    return NULL;
  }
  else {
    for (listnode_t* node = list->head->next; node != NULL; node = node ->next){
      if (strcmp(node->key,key)==0){
        return node->data;
      }
    }
    return NULL;
  }
}

/** list_insert inserts the given key/data pair into the given list
* It takes in a list, a key and a data input
* If the list is NULL, return NULL
* If the key already exists in the list, return false.
* If the key is not in the list, insert the key/data pair and return true
*/

bool list_insert(list_t* list, char* key, void *data)
{
  if (list != NULL) {
    if (list_find(list,key) == NULL){

        // Call helper function new node to allocate memory for the key/data pair

        listnode_t *node = newNode(key,data);
        listnode_t *indexNode = list->head; 

        // Search for the last node in the list   
        while(indexNode->next != NULL){
          indexNode = indexNode->next;
        }
        // Insert the new node after the current last node in the list
        node->next = indexNode->next;
        indexNode->next = node;
        return true;
        }
        else{
          return false;
          }
        }
    else{
      return false;
    }
  }


/** newNode allocates memory for a given key/data pair
* It takes in a key and a data input
* If the node or the node->key is allocated incorrectly, return NULL
* Else, return the node
*/

listnode_t* newNode(char* key, void* data){

  listnode_t* node = calloc(CALLOC_INT_LIST,sizeof(listnode_t)); // allocate memory for the node structure
  if (node == NULL)
    return NULL;
  else{
    node->next = NULL; //  assigns the next pointer to null
    node->key = calloc(CALLOC_INT_LIST,strlen(key)+1); // allocates memory for the node->key structure 
    if (node->key == NULL){ // if its incorrectly assigned, free the node and return null
      free(node);
      return NULL;
    }
    else{
      strcpy(node->key,key); // copies the string from the given key to the node->key
      node->data = data;
    }
  }
  return node; 
}

/** list_delete function takes a given list and deletes all the nodes in the list and the list itself 
* It takes a list as a parameter
* It calls the list_delete_helper function to delete all the elements in the list from the head node in the list
* If the list does not exist, return NULL
*/

void list_delete(list_t* list)
{
  if (list == NULL){
    return;
  } else if (list->head == NULL){
    return;
  } else {
    list_delete_helper(list, list->head);
  }

}

/** list_delete_helper is a helper function which deletes a given list from its head node
* If the given list or the headnode is NULL, return NULL.
* Else, call freeNode to free each node in the list and free the entire list.
*/

void list_delete_helper(list_t* list, listnode_t* listnode)
{ 

  if (list == NULL){
    return; // allow error to silently fall
  }
  
  // local variables for the nodes to cycle through 

  listnode_t* node = listnode; 
  listnode_t* tempnode = node; 

  if (node != NULL){

    // loop over the entire list and deletes from the head 
    while (node != NULL){
      tempnode = node;
      node = node->next;
      
      // call itemdelete 'destructor' function to delete the data from tempnode. 

      if (list->itemdelete != NULL)
        (*list->itemdelete)(tempnode->data);
      freeNode(tempnode); // call freeNode on tempNode to delete the node and its elements
    }
    free(list); // deletes the entire list
  }

  else {
    return; // print error message and allow error to silently fall
  }
}

/** freeNode deletes the structure and the elements in each node
* Takes in a listnode
* If the listnode equals to NULL, return NULL
* Else, free the node->key as well as the node.
*/

void freeNode(listnode_t* listnode)
{
  listnode_t* node = listnode;
  if (node != NULL) { // checks to see if node has a memory allocation
    if (node->key != NULL) // checks to see if node->key has a memory allocation
      free(node->key);
    free(node);
  }
  else{
    return;
  }
}


void listnode_delete(list_t* list, char* key)
{
    if (list == NULL){
      ;
    }
    else {
      for (listnode_t* node = list->head->next; node != NULL; node = node ->next){
        if (strcmp(node->key,key)==0){
          freeNode(node);
        }
      }
    }
}


/* Iterate over all items in list (in undefined order):
 * call itemfunc for each item, passing (arg, key, data).
 */
void list_iterate(list_t *list, void (*itemfunc)(void *arg, char *key, void *data), void *arg)
{
  if (list != NULL && itemfunc != NULL) {
    for (listnode_t* node = list->head; node != NULL; node = node->next){
        (*itemfunc)(arg, node->key, node->data);  
    }
  }
}





/** @file libpriqueue.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "libpriqueue.h"


/**
  Initializes the priqueue_t data structure.
  
  Assumtions
    - You may assume this function will only be called once per instance of priqueue_t
    - You may assume this function will be the first function called using an instance of priqueue_t.
  @param q a pointer to an instance of the priqueue_t data structure
  @param comparer a function pointer that compares two elements.
  See also @ref comparer-page
 */
void priqueue_init(priqueue_t *q, int(*comparer)(const void *, const void *))
{
  q->values = malloc(sizeof(void*) * 10);
  q->size =0;
  q->capacity = 10;
  q->comparer = comparer;
}


/**
  Insert the specified element into this priority queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr a pointer to the data to be inserted into the priority queue
  @return The zero-based index where ptr is stored in the priority queue, where 0 indicates that ptr was stored at the front of the priority queue.
 */
int priqueue_offer(priqueue_t *q, void *ptr)
{
  //resizing it if needed
  if(q->size >= q->capacity){
    q->capacity *= 2;
    q->values = realloc(q->values, sizeof(void*) * q-> capacity);
  }

  //insert based on priority as per comparator
  int i;
  for(i = q->size; i>0 && q->comparer(q->values[i - 1], ptr) > 0; i--){
    q->values[i] = q->values[i-1];
  }

    q->values[i] = ptr;
    q->size++;

	return i;
}


/**
  Retrieves, but does not remove, the head of this queue, returning NULL if
  this queue is empty.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return pointer to element at the head of the queue
  @return NULL if the queue is empty
 */
void *priqueue_peek(priqueue_t *q)
{
  if(q->size == 0){
    return NULL;
  }
	
  return q->values[0];
}


/**
  Retrieves and removes the head of this queue, or NULL if this queue
  is empty.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return the head of this queue
  @return NULL if this queue is empty
 */
void *priqueue_poll(priqueue_t *q)
{
  if(q->size ==0){
    return NULL;
  }

  void* frontElement = q->values[0];
  for(int i = 1; i< q->size; i++){
    q->values[i-1] = q->values[i];
  }
  q->size--;
	return frontElement;
}


/**
  Returns the element at the specified position in this list, or NULL if
  the queue does not contain an index'th element.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of retrieved element
  @return the index'th element in the queue
  @return NULL if the queue does not contain the index'th element
 */
void *priqueue_at(priqueue_t *q, int index)
{
  if(index<0 || index >= q->size){
    return NULL;
  }

	return q->values[index];
}


/**
  Removes all instances of ptr from the queue. 
  
  This function should not use the comparer function, but check if the data contained in each element of the queue is equal (==) to ptr.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr address of element to be removed
  @return the number of entries removed
 */
int priqueue_remove(priqueue_t *q, void *ptr)
{
  int removed =0;
  for(int i =0; i< q->size; i++){
    if(q->values[i] == ptr){
      for(int j =i; j< q->size -1; j++){
        q->values[j] = q->values[j+1];
      }
      q->size = q->size -1;
      removed += 1;
      i--;
    }
  }

	return removed;
}


/**
  Removes the specified index from the queue, moving later elements up
  a spot in the queue to fill the gap.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of element to be removed
  @return the element removed from the queue
  @return NULL if the specified index does not exist
 */
void *priqueue_remove_at(priqueue_t *q, int index)
{
	if (index < 0 || index >= q->size) return NULL;
  
  void* removedElement = q->values[index];
  for (int i = index; i < q->size - 1; i++) {
    q->values[i] = q->values[i + 1];
  }
  
  q->size--;

  return removedElement;
}


/**
  Return the number of elements in the queue.
 
  @param q a pointer to an instance of the priqueue_t data structure
  @return the number of elements in the queue
 */
int priqueue_size(priqueue_t *q)
{
	return q->size;
}


/**
  Destroys and frees all the memory associated with q.
  
  @param q a pointer to an instance of the priqueue_t data structure
 */
void priqueue_destroy(priqueue_t *q)
{
  free(q->values);
  q->values = NULL;
  q->size = 0;
  q->capacity = 0;
  q->comparer = NULL;
}

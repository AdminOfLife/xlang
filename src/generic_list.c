#include "generic_list.h"

/* Function to add a node at the beginning of Linked List.
   This function expects a pointer to the data to be added
   and size of the data type */
void push(struct list_t** head_ref, void *new_data, size_t data_size)
{
    // Allocate memory for node
    struct list_t* new_node = (struct list_t*)malloc(sizeof(struct list_t));

    new_node->data  = malloc(data_size);
    new_node->next = (*head_ref);

    // Copy contents of new_data to newly allocated memory.
    // Assumption: char takes 1 byte.
	{	
		int i;
		for (i=0; i<data_size; i++)
			*(char *)((char*)new_node->data + i) = *(char *)((char *)new_data + i);
	}
    // Change head pointer as new node is added at the beginning
    (*head_ref)    = new_node;
}

/* Function to print nodes in a given linked list. fpitr is used
   to access the function to be used for printing current node data.
   Note that different data types need different specifier in printf() */
void printList(struct list_t *node, void (*fptr)(void *))
{
    while (node != NULL)
    {
        (*fptr)(node->data);
        node = node->next;
    }
}



void *foreach_element(struct list_t *node, void *data, void *(*fptr)(const void *, const void*))
{
    while (node != NULL)
    {
        if ((*fptr)(node->data, data)){
          return node->data;
        }
        node = node->next;
    }
    return 0;

}
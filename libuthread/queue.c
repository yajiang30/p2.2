#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

struct node {
	void* data;
	struct node* pre;
	struct node* next;
};

struct queue {
	struct node* head;
	struct node* tail;
	int len;
};

queue_t queue_create(void)
{
	struct queue* queue = (struct queue*)malloc(sizeof(struct queue));
	if (!queue) {
		return NULL;
	}
	// Initialize fields
	queue->head = NULL;
	queue->tail = NULL;
	queue->len = 0;
	return queue;
}

struct node* node_create(void *data){
	struct node* node = (struct node*)malloc(sizeof(struct node));
	if(!node){
		return NULL;
	}
	// Initialize fields
	node->data = data;
	node->pre = NULL;
	node->next = NULL;
	return node;
}

// This function cost time complexity O(1)
void delete_node(queue_t queue, struct node* node){
	if(node == NULL){
		return;
	}
	if(queue->head == node){
		// If the node is the header node
		queue->head = node->next;
		if (queue->head != NULL){
			queue->head->pre = NULL;
		}
	}else if(queue->tail == node){
		// If the node is the tail node
		queue->tail = node->pre;
		if(queue->tail != NULL){
			queue->tail->next = NULL;
		}
	}else{
		// If the node is an internel node
		struct node* pre = node->pre;
		struct node* next = node->next;
		pre->next = next;
		next->pre = pre;
	}
	queue->len--;
	free(node);
	if (queue->len == 0){
		queue->head = queue->tail = NULL;
	}
}

int queue_destroy(queue_t queue)
{
	// Delete header node until the queue is empty
	while(queue->len){
		delete_node(queue, queue->head);
	}
	free(queue);
	return 0;
}

int queue_enqueue(queue_t queue, void *data)
{
	if(!queue){
		return -1;
	}
	// Create node
	struct node* node = node_create(data);
	if (!node) {
		return -1;
	}
	if (queue->len == 0){
		queue->head = queue->tail = node;
		queue->len++;
		return 0;
	}
	// Add the node to the end of queue
	queue->tail->next = node;
	node->pre = queue->tail;
	queue->tail = node;
	queue->len++;
	return 0;
}

int queue_dequeue(queue_t queue, void **data)
{
	if(!queue){
		return -1;
	}
	if(queue->len == 0){
		return -1;
	}
	*data = queue->head->data;
	// Remove the header node from the queue
	delete_node(queue, queue->head);
	return 0;
}

int queue_delete(queue_t queue, void *data)
{
	if(!queue){
		return -1;
	}
	// Traverse the queue and find the data
	struct node* cur = queue->head;
	while (cur) {
		if (cur->data == data) {
			// Delete node
			delete_node(queue, cur);
			return 0;
		}
		// Go to the next node
		cur = cur->next;
	}
	return -1;
}

int queue_iterate(queue_t queue, queue_func_t func)
{
	// Traverse the queue
	struct node* cur = queue->head;
	while (cur) {
		struct node* next = cur->next;
		// Call the function
		func(queue, cur->data);
		// Go to the next node
		cur = next;
	}
	return 0;
}

int queue_length(queue_t queue)
{
	return queue->len;
}


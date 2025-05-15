#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <queue.h>

#define TEST_ASSERT(assert)				\
do {									\
	printf("ASSERT: " #assert " ... ");	\
	if (assert) {						\
		printf("PASS\n");				\
	} else	{							\
		printf("FAIL\n");				\
		exit(1);						\
	}									\
} while(0)

/* Create */
void test_create(void)
{
	fprintf(stderr, "*** TEST create ***\n");

	TEST_ASSERT(queue_create() != NULL);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void)
{
	int data = 3, *ptr;
	queue_t q;

	fprintf(stderr, "*** TEST queue_simple ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_dequeue(q, (void**)&ptr);
	TEST_ASSERT(ptr == &data);
}

void test_enqueue(void) {
	fprintf(stderr, "*** TEST enqueue ***\n");
	int data[5] = {1, 2, 3, 4, 5};
	queue_t q = queue_create();
	for(int i=0;i<5;i++){
		queue_enqueue(q, &data[i]);
	}
	int* d = NULL;
	for(int i=0;i<5;i++){
		queue_dequeue(q, (void**)&d);
		TEST_ASSERT(d == &data[i]);
		TEST_ASSERT(queue_length(q) == (5 - i - 1));
	}
}

void test_delete_node(queue_t queue, void *data){
	queue_delete(queue, data);
}

void test_queue_delete_all(void){
	fprintf(stderr, "*** TEST queue_delete_all ***\n");
	int data[5] = {1, 2, 3, 4, 5};
	queue_t q = queue_create();
	for(int i=0;i<5;i++){
		queue_enqueue(q, &data[i]);
	}
	TEST_ASSERT(queue_length(q) == 5);
	queue_iterate(q, test_delete_node);
	TEST_ASSERT(queue_length(q) == 0);
}

void test_modify_data(queue_t queue, void *data){
	if(queue_length(queue) == 0){
		return;
	}
	int *a = (int*)data;
	*a *= 2;
}

void test_queue_iterate(void){
	fprintf(stderr, "*** TEST queue_iterate ***\n");
	int data_reserved[5] = {1, 2, 3, 4, 5};
	int data[5] = {1, 2, 3, 4, 5};
	queue_t q = queue_create();
	for(int i=0;i<5;i++){
		queue_enqueue(q, &data[i]);
	}
	queue_iterate(q, test_modify_data);
	int* d = NULL;
	for(int i=0;i<5;i++){
		queue_dequeue(q, (void**)&d);
		TEST_ASSERT(*d == data_reserved[i] * 2);
	}
}

int main(void)
{
	test_create();
	test_queue_simple();
	test_enqueue();
	test_queue_delete_all();
	test_queue_iterate();

	return 0;
}

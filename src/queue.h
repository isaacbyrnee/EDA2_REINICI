#ifndef QUEUE_H
#define QUEUE_H

#define MAX_QUERY_LEN 100
#define MAX_HISTORY 3

typedef struct Node {
  char query[MAX_QUERY_LEN];
  struct Node *next;
} Node;

typedef struct {
  int size;
  Node *front;
  Node *rear;
} Queue;

Queue *create_queue();
void enqueue(Queue *q, const char *query);
void print_queue(Queue *q);
void free_queue(Queue *q);

#endif

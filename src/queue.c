#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Queue *create_queue() {
  Queue *q = malloc(sizeof(Queue));
  q->size = 0;
  q->front = q->rear = NULL;
  return q;
}

void enqueue(Queue *q, const char *query) {
  Node *new_node = malloc(sizeof(Node));
  strncpy(new_node->query, query, MAX_QUERY_LEN);
  new_node->next = NULL;

  if (q->rear) {
    q->rear->next = new_node;
  } else {
    q->front = new_node;
  }

  q->rear = new_node;
  q->size++;

  // Elimina l’element més antic si n’hi ha més de 3
  if (q->size > MAX_HISTORY) {
    Node *old = q->front;
    q->front = q->front->next;
    free(old);
    q->size--;
  }
}

void print_queue(Queue *q) {
  printf("\n\n          ******* cerques recents ********\n");
  Node *current = q->front;
  while (current) {
    printf("          * %s *\n", current->query);
    current = current->next;
  }
  printf("          ********************************\n\n");
}

void free_queue(Queue *q) {
  Node *current = q->front;
  while (current) {
    Node *next = current->next;
    free(current);
    current = next;
  }
  free(q);
}

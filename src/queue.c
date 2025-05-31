// - - - queue.c - - - //

#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//---FUNCIONS D'INICIALITZACIÓ---//

// Crea i inicialitza una nova cua.
Queue *create_queue() {
  Queue *q = malloc(sizeof(Queue));
  if (!q) {
    // Error al asignar memòria
    return NULL;
  }
  q->size = 0;
  q->front = q->rear = NULL;
  return q;
}

//---FUNCIONS D'OPERACIÓ DE COLA---//

// Afegeix un element (consulta) a la cua.
void enqueue(Queue *q, const char *query) {
  Node *new_node = malloc(sizeof(Node));
  if (!new_node) {
    // Error al asignar memòria
    return;
  }
  strncpy(new_node->query, query, MAX_QUERY_LEN);
  new_node->query[MAX_QUERY_LEN - 1] = '\0'; // Assegura terminació nul·la
  new_node->next = NULL;

  if (q->rear) {
    q->rear->next = new_node;
  } else {
    q->front = new_node;
  }
  q->rear = new_node;
  q->size++;

  // Elimina l'element més antic si la cua excedeix el límit d'historial.
  if (q->size > MAX_HISTORY) {
    Node *old = q->front;
    q->front = q->front->next;
    free(old);
    q->size--;
  }
}

//---FUNCIONS DE VISUALITZACIÓ---//

// Imprimeix el contingut de la cua (historial de cerques).
void print_queue(Queue *q) {
  printf("\n\n           ******* cerques recents ********\n");
  Node *current = q->front;
  while (current) {
    printf("           * %s *\n", current->query);
    current = current->next;
  }
  printf("           ********************************\n\n");
}

//---FUNCIONS D'ALLIBERAMENT DE MEMÒRIA (FREE)---//

// Allibera tota la memòria que haviem assignat prèviament a la cua i als nodes
void free_queue(Queue *q) {
  Node *current = q->front;
  while (current) {
    Node *next = current->next;
    free(current);
    current = next;
  }
  free(q);
}
// - - - queue.h - - - //

#ifndef QUEUE_H
#define QUEUE_H

//---DEFINICIONS DE CONSTANTS---//

#define MAX_QUERY_LEN 100 // Longitud màxima per a una cadena de consulta.
#define MAX_HISTORY 3 // Nombre màxim d'elements a mantenir a l'historial (cua).

//---ESTRUCTURES DE DADES DE LA CUA---//

// Node individual de la cua, conté una consulta i un punter al següent node.
typedef struct Node {
  char query[MAX_QUERY_LEN];
  struct Node *next;
} Node;

// Estructura principal de la cua, conté la mida, i punters al front i al final.
typedef struct {
  int size;
  Node *front; // Punter al primer element de la cua.
  Node *rear;  // Punter a l'últim element de la cua.
} Queue;

//---DECLARACIÓ DE FUNCIONS DE LA CUA---//

// Crea i inicialitza una nova cua.
Queue *create_queue();
// Afegeix una consulta a la cua.
void enqueue(Queue *q, const char *query);
// Imprimeix el contingut de la cua.
void print_queue(Queue *q);
// Allibera tota la memòria assignada a la cua.
void free_queue(Queue *q);

#endif
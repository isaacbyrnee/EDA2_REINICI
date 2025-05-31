// - - - document_graph.h - - - //

#ifndef DOCUMENT_GRAPH_H
#define DOCUMENT_GRAPH_H

#include <stdbool.h>
#include <stdlib.h>

// --- ESTRUCTURES DE DADES DEL GRAF --- //

// Node de la llista d'adjacència (representa un enllaç a un altre document)
typedef struct AdjNode {
  int doc_id;
  struct AdjNode *next;
} AdjNode;

// Node del graf (representa un document)
typedef struct GraphNode {
  int doc_id;
  int indegree;           // Nombre d'enllaços entrants
  AdjNode *adj_list;      // Llista d'adjacència (enllaços sortints)
  struct GraphNode *next; // Per a la llista de nodes en el graf
} GraphNode;

// Estructura principal del graf
typedef struct DocumentGraph {
  GraphNode *nodes; // Llista que conté tots els nodes del graf
  int num_nodes;
} DocumentGraph;

// --- DECLARACIÓ DE FUNCIONS DEL GRAF --- //

// Funcions d'inicialització
DocumentGraph *graph_init();

// Funcions de modificació i construcció del graf
void graph_add_node(DocumentGraph *graph, int doc_id);
void graph_add_edge(DocumentGraph *graph, int from_doc_id, int to_doc_id);

// Funcions de consulta
int graph_get_indegree(DocumentGraph *graph, int doc_id);

// Funcions de cerca i comprovació
GraphNode *graph_find_node(DocumentGraph *graph, int doc_id);
bool graph_node_exists(DocumentGraph *graph, int doc_id);

// Funcions d'alliberament de memòria (free)
void graph_free(DocumentGraph *graph);

#endif // DOCUMENT_GRAPH_H
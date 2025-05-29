#ifndef DOCUMENT_GRAPH_H
#define DOCUMENT_GRAPH_H

#include <stdbool.h>
#include <stdlib.h>

// Nodo de la lista de adyacencia (representa un enlace a otro documento)
typedef struct AdjNode {
  int doc_id;
  struct AdjNode *next;
} AdjNode;

// Nodo del grafo (representa un documento)
typedef struct GraphNode {
  int doc_id;
  int indegree;           // Número de enlaces entrantes
  AdjNode *adj_list;      // Lista de adyacencia (enlaces salientes)
  struct GraphNode *next; // Para la lista de nodos en el grafo
} GraphNode;

// Estructura principal del grafo
typedef struct DocumentGraph {
  GraphNode *nodes; // Lista de todos los nodos del grafo
  int num_nodes;
} DocumentGraph;

// --- Funciones del Grafo ---
DocumentGraph *graph_init();
void graph_add_node(DocumentGraph *graph, int doc_id);
void graph_add_edge(DocumentGraph *graph, int from_doc_id, int to_doc_id);
int graph_get_indegree(DocumentGraph *graph, int doc_id);
GraphNode *graph_find_node(DocumentGraph *graph, int doc_id);
bool graph_node_exists(DocumentGraph *graph,
                       int doc_id); // Nueva función auxiliar
void graph_free(DocumentGraph *graph);

#endif // DOCUMENT_GRAPH_H
#include "document_graph.h"
#include <stdio.h>
#include <stdlib.h>

// Inicializa un nuevo grafo de documentos
DocumentGraph *graph_init() {
  DocumentGraph *graph = (DocumentGraph *)malloc(sizeof(DocumentGraph));
  if (!graph) {
    fprintf(stderr, "Error: Fallo al asignar memoria para DocumentGraph.\n");
    return NULL;
  }
  graph->nodes = NULL;
  graph->num_nodes = 0;
  return graph;
}

// Encuentra un nodo en el grafo por su ID
GraphNode *graph_find_node(DocumentGraph *graph, int doc_id) {
  if (!graph)
    return NULL;
  GraphNode *current = graph->nodes;
  while (current != NULL) {
    if (current->doc_id == doc_id) {
      return current;
    }
    current = current->next;
  }
  return NULL;
}

// Comprueba si un nodo existe en el grafo
bool graph_node_exists(DocumentGraph *graph, int doc_id) {
  return graph_find_node(graph, doc_id) != NULL;
}

// Añade un nodo al grafo (si no existe ya)
void graph_add_node(DocumentGraph *graph, int doc_id) {
  if (!graph)
    return;

  if (graph_find_node(graph, doc_id) != NULL) {
    return; // El nodo ya existe
  }

  GraphNode *new_node = (GraphNode *)malloc(sizeof(GraphNode));
  if (!new_node) {
    fprintf(stderr, "Error: Fallo al asignar memoria para GraphNode.\n");
    return;
  }
  new_node->doc_id = doc_id;
  new_node->indegree = 0;
  new_node->adj_list = NULL;
  new_node->next = graph->nodes; // Añadir al principio de la lista de nodos
  graph->nodes = new_node;
  graph->num_nodes++;
}

// Añade una arista dirigida de from_doc_id a to_doc_id
void graph_add_edge(DocumentGraph *graph, int from_doc_id, int to_doc_id) {
  if (!graph)
    return;

  GraphNode *from_node = graph_find_node(graph, from_doc_id);
  GraphNode *to_node = graph_find_node(graph, to_doc_id);

  if (from_node == NULL || to_node == NULL) {
    // Uno o ambos nodos no existen, no se puede añadir la arista
    // fprintf(stderr, "Advertencia: No se pudo añadir arista de %d a %d. Uno o
    // ambos nodos no existen.\n", from_doc_id, to_doc_id);
    return;
  }

  // Añadir a la lista de adyacencia de from_node
  AdjNode *new_adj_node = (AdjNode *)malloc(sizeof(AdjNode));
  if (!new_adj_node) {
    fprintf(stderr, "Error: Fallo al asignar memoria para AdjNode.\n");
    return;
  }
  new_adj_node->doc_id = to_doc_id;
  new_adj_node->next =
      from_node->adj_list; // Añadir al principio de la lista de adyacencia
  from_node->adj_list = new_adj_node;

  // Incrementar el indegree del nodo destino
  to_node->indegree++;
}

// Obtiene el indegree de un documento
int graph_get_indegree(DocumentGraph *graph, int doc_id) {
  GraphNode *node = graph_find_node(graph, doc_id);
  if (node != NULL) {
    return node->indegree;
  }
  return 0; // Si el nodo no existe, su indegree es 0
}

// Libera la memoria del grafo
void graph_free(DocumentGraph *graph) {
  if (!graph)
    return;

  GraphNode *current_node = graph->nodes;
  while (current_node != NULL) {
    GraphNode *next_node = current_node->next;
    AdjNode *current_adj = current_node->adj_list;
    while (current_adj != NULL) {
      AdjNode *next_adj = current_adj->next;
      free(current_adj);
      current_adj = next_adj;
    }
    free(current_node);
    current_node = next_node;
  }
  free(graph);
}
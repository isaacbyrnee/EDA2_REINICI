// - - - document_graph.c - - - //

#include "document_graph.h"
#include <stdio.h>
#include <stdlib.h>

//---FUNCIONS D'INICIALITZACIÓ---//

// Inicialitza un graf nou
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

//---FUNCIONS DE CERCA I COMPROVACIÓ---//

// Donat un ID, ens permet trobar un node i per tant un document i info
// rellevant
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

// Funció auxiliar que serveix per comprovar si un node ja existeix
bool graph_node_exists(DocumentGraph *graph, int doc_id) {
  return graph_find_node(graph, doc_id) != NULL;
}

//---FUNCIONS DE MODIFICACIÓ I CONSTRUCCIÓ DEL GRAF---//

// Per afegir un node al graf, sempre que aquest no existeixi
void graph_add_node(DocumentGraph *graph, int doc_id) {
  if (!graph)
    return;

  if (graph_find_node(graph, doc_id) != NULL) {
    return; // El node ja existeix
  }

  GraphNode *new_node = (GraphNode *)malloc(sizeof(GraphNode));
  if (!new_node) {
    fprintf(stderr, "Error: Fallo al asignar memoria para GraphNode.\n");
    return;
  }
  new_node->doc_id = doc_id;
  new_node->indegree = 0;
  new_node->adj_list = NULL;
  new_node->next = graph->nodes; // Afegeix el nou node al principi de la llista
                                 // que recopila els nodesja existents
  graph->nodes = new_node;
  graph->num_nodes++;
}

// Connecta dos nodes, afegint una aresta entre ells
void graph_add_edge(DocumentGraph *graph, int from_doc_id, int to_doc_id) {
  if (!graph)
    return;

  GraphNode *from_node = graph_find_node(graph, from_doc_id);
  GraphNode *to_node = graph_find_node(graph, to_doc_id);

  if (from_node == NULL || to_node == NULL) {
    // No podem afegir una aresta si un dels dos nodes no existeix
    return;
  }

  // Per calculs posteriors, afegim aquests nous nodes a la llista d'adjacència
  // de cadascún d'ells
  AdjNode *new_adj_node = (AdjNode *)malloc(sizeof(AdjNode));
  if (!new_adj_node) {
    fprintf(stderr, "Error: Fallo al asignar memoria para AdjNode.\n");
    return;
  }
  new_adj_node->doc_id = to_doc_id;
  new_adj_node->next =
      from_node->adj_list; // L'afegim al principi de la llista d'adjacència
  from_node->adj_list = new_adj_node;

  // Incrementem en un el numero de nodes incidents en el node
  to_node->indegree++;
}

//---FUNCIONS DE CONSULTA---//

// Funció molt útil que permet agafar la quantitat de nodes incidents a un node
int graph_get_indegree(DocumentGraph *graph, int doc_id) {
  GraphNode *node = graph_find_node(graph, doc_id);
  if (node != NULL) {
    return node->indegree;
  }
  return 0; // Clarament, si el node no existeix aquest numero serà 0
}

//---FUNCIONS D'ALLIBERAMENT DE MEMÒRIA (FREE)---//

// Per assegurar que no hi ha fuites de memoria creem aquesta funció que
// allibera la memoria del graf
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
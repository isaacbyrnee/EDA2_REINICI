// // document_graph.c
// #include "document_graph.h"
// #include <stdio.h>  // Para printf en debugging o errores
// #include <stdlib.h> // Para malloc, free
// #include <string.h> // Para strdup, etc.

// // Función auxiliar para obtener un vértice o crearlo si no existe
// // Esto es útil si los IDs de documentos no son secuenciales o si no queremos
// // inicializar un array gigante para IDs muy dispersos.
// // Para el lab, con IDs 0.txt, 1.txt, etc., podemos usar los IDs directamente
// // como índices después de asegurar un tamaño adecuado para el array de
// // vertices.
// GraphVertex *get_or_create_vertex(DocumentGraph *graph, int doc_id) {
//   // Si los IDs son secuenciales y no hay muchos huecos, podemos usarlos como
//   // índices. De lo contrario, necesitaríamos un hashmap interno para mapear
//   // doc_id a índices. Para simplificar aquí, asumimos que graph->vertices es
//   lo
//   // suficientemente grande para albergar doc_id como índice directo.
//   if (doc_id < 0 || doc_id > graph->max_doc_id) {
//     fprintf(stderr, "Error: Document ID %d fuera de rango.\n", doc_id);
//     return NULL;
//   }

//   if (graph->vertices[doc_id] == NULL) {
//     GraphVertex *new_vertex = (GraphVertex *)malloc(sizeof(GraphVertex));
//     if (!new_vertex) {
//       fprintf(stderr, "Error: Fallo al asignar memoria para GraphVertex.\n");
//       return NULL;
//     }
//     new_vertex->doc_id = doc_id;
//     new_vertex->adj_list = NULL;
//     new_vertex->indegree = 0;
//     graph->vertices[doc_id] = new_vertex;
//     // Solo incrementamos num_documents si realmente es un documento nuevo en
//     el
//     // grafo Si el grafo se construye con todos los documentos al inicio,
//     // num_documents ya estaría fijado. Aquí lo mantenemos si se añaden
//     vertices
//     // de forma dinámica.
//     graph->num_documents++;
//   }
//   return graph->vertices[doc_id];
// }

// DocumentGraph *graph_init(int max_id) {
//   DocumentGraph *graph = (DocumentGraph *)malloc(sizeof(DocumentGraph));
//   if (!graph) {
//     fprintf(stderr, "Error: Fallo al asignar memoria para DocumentGraph.\n");
//     return NULL;
//   }

//   // max_id + 1 para incluir el documento con el ID más alto (si los IDs
//   // empiezan en 0)
//   graph->max_doc_id = max_id;
//   graph->vertices = (GraphVertex **)calloc(max_id + 1, sizeof(GraphVertex
//   *)); if (!graph->vertices) {
//     fprintf(stderr,
//             "Error: Fallo al asignar memoria para vertices del grafo.\n");
//     free(graph);
//     return NULL;
//   }
//   graph->num_documents = 0; // Se incrementará al añadir vértices reales
//   return graph;
// }

// void graph_add_edge(DocumentGraph *graph, int source_doc_id,
//                     int target_doc_id) {
//   if (!graph)
//     return;

//   // Asegurarse de que ambos vértices existan en el grafo
//   GraphVertex *source_vertex = get_or_create_vertex(graph, source_doc_id);
//   GraphVertex *target_vertex = get_or_create_vertex(graph, target_doc_id);

//   if (!source_vertex || !target_vertex) {
//     // Error ya reportado por get_or_create_vertex
//     return;
//   }

//   // Añadir el target_doc_id a la lista de adyacencia del source_doc_id
//   AdjNode *new_adj_node = (AdjNode *)malloc(sizeof(AdjNode));
//   if (!new_adj_node) {
//     fprintf(stderr, "Error: Fallo al asignar memoria para AdjNode.\n");
//     return;
//   }
//   new_adj_node->target_doc_id = target_doc_id;
//   new_adj_node->next =
//       source_vertex->adj_list; // Añadir al principio de la lista
//   source_vertex->adj_list = new_adj_node;

//   // Incrementar el indegree del nodo destino
//   target_vertex->indegree++;
// }

// int graph_get_indegree(DocumentGraph *graph, int document_id) {
//   if (!graph || document_id < 0 || document_id > graph->max_doc_id ||
//       graph->vertices[document_id] == NULL) {
//     return -1; // Documento no encontrado o ID fuera de rango
//   }
//   return graph->vertices[document_id]->indegree;
// }

// void graph_free(DocumentGraph *graph) {
//   if (!graph)
//     return;

//   for (int i = 0; i <= graph->max_doc_id; i++) {
//     GraphVertex *vertex = graph->vertices[i];
//     if (vertex) {
//       AdjNode *current_adj_node = vertex->adj_list;
//       while (current_adj_node) {
//         AdjNode *next_adj_node = current_adj_node->next;
//         free(current_adj_node);
//         current_adj_node = next_adj_node;
//       }
//       free(vertex); // Libera el vértice en sí
//     }
//   }
//   free(graph->vertices); // Libera el array de punteros a vértices
//   free(graph);           // Libera la estructura del grafo
// }

// // Implementación de build_document_graph
// void build_document_graph(DocumentList *all_documents, DocumentGraph *graph)
// {
//   if (!all_documents || !graph)
//     return;

//   // Primer paso: asegurar que todos los documentos originales tengan un
//   vértice
//   // en el grafo, y si es necesario, establecer num_documents correctamente.
//   // Esto es importante porque un documento puede no tener enlaces salientes,
//   // pero puede ser enlazado por otros (es decir, tener un indegree > 0).
//   Document *current_doc = all_documents->first_document;
//   int actual_max_id =
//       0; // Para el caso de que all_documents->size no sea el max_id
//   while (current_doc != NULL) {
//     if (current_doc->id > actual_max_id) {
//       actual_max_id = current_doc->id;
//     }
//     // Asegurarse de que el vértice existe, esto también inicializará su
//     // indegree a 0
//     get_or_create_vertex(graph, current_doc->id);
//     current_doc = current_doc->next_document;
//   }
//   // Si el graph_init se hizo con un max_id menor, podría ser problemático.
//   // Lo ideal es que graph_init sepa el max_id real de antemano.
//   // O que get_or_create_vertex sea más robusto con un hashmap interno si IDs
//   // son dispersos. Para esta implementación, se asume que graph->max_doc_id
//   es
//   // suficiente.

//   // Segundo paso: Recorrer cada documento y añadir sus enlaces como aristas
//   current_doc = all_documents->first_document;
//   while (current_doc != NULL) {
//     Link *current_link = current_doc->linklist->first;
//     while (current_link != NULL) {
//       graph_add_edge(graph, current_doc->id, current_link->id);
//       current_link = current_link->link_next;
//     }
//     current_doc = current_doc->next_document;
//   }
// }
// // document_graph.h
// #ifndef DOCUMENT_GRAPH_H
// #define DOCUMENT_GRAPH_H

// #include <stdbool.h>
// #include <stdlib.h>

// // Necesitamos conocer Document para asociar los nodos del grafo con sus IDs
// // No incluimos "documents.h" completo para evitar dependencias circulares.
// // Una declaración forward de la estructura Document es suficiente si solo
// // vamos a usar el ID y no necesitamos sus campos internos aquí.
// // Sin embargo, para fines de construcción del grafo, necesitaremos
// // la LinkList de Document, así que sí incluiremos "documents.h"
// #include "documents.h" // Se necesita para Document y LinkList

// // Un nodo en la lista de adyacencia que representa un enlace saliente
// typedef struct AdjNode {
//   int target_doc_id; // El ID del documento al que este nodo enlaza
//   struct AdjNode *next;
// } AdjNode;

// // Estructura para cada vértice (documento) en el grafo
// typedef struct GraphVertex {
//   int doc_id;        // El ID del documento
//   AdjNode *adj_list; // Lista de adyacencia de documentos a los que enlaza
//   int indegree;      // El número de enlaces que apuntan a este documento
//   // Podrías añadir un puntero al Document* original si lo necesitas aquí
//   // Document *original_document;
// } GraphVertex;

// // La estructura principal del grafo de documentos
// typedef struct DocumentGraph {
//   GraphVertex **vertices; // Un array de punteros a GraphVertex
//   int num_documents; // El número total de documentos/vértices en el grafo
//   // Un mapeo de doc_id a índice en el array 'vertices' para acceso rápido
//   // Podrías usar un array simple si los IDs son secuenciales desde 0
//   // o un hashmap si los IDs son dispersos y grandes.
//   // Para esta implementación, asumiremos que los doc_id se pueden mapear
//   // a índices directos (0 a N-1), o que gestionamos un array grande
//   // o un hashmap para IDs dispersos.
//   // Dada la naturaleza de los documentos del Lab (0.txt, 1.txt, etc.),
//   // asumiremos que los IDs pueden ser usados como índices directamente,
//   // o que el número de documentos total es el máximo ID + 1.
//   int max_doc_id; // El ID máximo de un documento para dimensionar el array
// } DocumentGraph;

// // --- Prototipos de funciones para el grafo ---

// /**
//  * @brief Inicializa una nueva estructura DocumentGraph.
//  * @param max_id El ID máximo de un documento en el dataset.
//  * @return Un puntero a la estructura DocumentGraph inicializada, o NULL en
//  caso
//  * de error.
//  */
// DocumentGraph *graph_init(int max_id);

// /**
//  * @brief Añade una arista dirigida desde source_doc_id a target_doc_id en el
//  * grafo. También incrementa el indegree de target_doc_id.
//  * @param graph El grafo al que añadir la arista.
//  * @param source_doc_id El ID del documento de origen.
//  * @param target_doc_id El ID del documento de destino.
//  */
// void graph_add_edge(DocumentGraph *graph, int source_doc_id, int
// target_doc_id);

// /**
//  * @brief Calcula y devuelve el indegree de un documento dado su ID.
//  * @param graph El grafo de documentos.
//  * @param document_id El ID del documento cuyo indegree se quiere obtener.
//  * @return El indegree del documento, o -1 si el documento no se encuentra en
//  el
//  * grafo.
//  */
// int graph_get_indegree(DocumentGraph *graph, int document_id);

// /**
//  * @brief Libera toda la memoria asociada con el DocumentGraph.
//  * @param graph El grafo a liberar.
//  */
// void graph_free(DocumentGraph *graph);

// // --- Funciones auxiliares para la construcción del grafo ---
// /**
//  * @brief Itera sobre todos los documentos cargados y construye el grafo.
//  * @param all_documents La lista de todos los documentos cargados.
//  * @param graph El grafo vacío a construir.
//  */
// void build_document_graph(DocumentList *all_documents, DocumentGraph *graph);

// #endif // DOCUMENT_GRAPH_H
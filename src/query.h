#ifndef QUERY_H
#define QUERY_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORD_LENGTH 100 // No parece usarse, pero si lo usas, bien.

// ¡IMPORTANTE! Incluir inverted_index.h AQUÍ para que InvertedIndex sea
// conocido
#include "documents.h" // También se necesita, ya que QueryItem_in_doc y document_search usan DocumentList/Document
#include "inverted_index.h" // <--- AÑADIR ESTA LÍNEA

typedef struct QueryItem {
  char *word;
  struct QueryItem *next_word;
} QueryItem;

typedef struct Query {
  int size;
  QueryItem *first_word;
} Query;

// --- Funciones para Query BASICS -----
Query *
InitQuery(char *search); // Añadir el argumento char *search a la declaración
bool QueryItem_in_doc(
    Query *query, Document *doc); // Si la mantienes, debería estar declarada
bool is_doc_already_in_list(DocumentList *list, int id);
void free_query(Query *q); // <--- DECLARAR ESTA FUNCIÓN AQUÍ

// ----- Busquedes amb QUERY i HASH -----
DocumentList *hash_document_search(DocumentList *docs, Query *query,
                                   InvertedIndex *index);
DocumentList *query_document_search(DocumentList *docs, Query *query);

#endif // QUERY_H
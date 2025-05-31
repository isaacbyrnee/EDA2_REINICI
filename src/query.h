#ifndef QUERY_H
#define QUERY_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define COMMON_DELIMITERS                                                      \
  " ,.!?;:[](){}\n" // Asegúrate que esta lista incluye *todos* los caracteres
                    // que quieres que actúen como separadores de palabras.

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
bool document_contains_all_query_words(Document *doc, Query *query);
bool is_doc_already_in_list(DocumentList *list, int id);
void free_query(Query *q); // <--- DECLARAR ESTA FUNCIÓN AQUÍ

// ----- Busquedes amb QUERY i HASH -----
DocumentList *linear_document_search(DocumentList *all_docs, Query *query);
DocumentList *inv_index_document_search(DocumentList *all_docs, Query *query,
                                        InvertedIndex *index);

#endif // QUERY_H
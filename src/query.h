// - - - query.h - - - //

#ifndef QUERY_H
#define QUERY_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define COMMON_DELIMITERS " ,.!?;:[](){}\n"

#include "documents.h"
#include "inverted_index.h"

// Estructura de QueryItem
typedef struct QueryItem {
  char *word;
  struct QueryItem *next_word;
} QueryItem;

// Estructura de la cerca
typedef struct Query {
  int size;
  QueryItem *first_word;
} Query;

// --- DECLARACIÓ DE FUNCIONS DE LA QUERY --- //

// Gestió de consultes (query)
Query *InitQuery(char *search);
void free_query(Query *q);

// Lògica de cerca i filtratge
bool document_contains_all_query_words(Document *doc, Query *query);
DocumentList *linear_document_search(DocumentList *all_docs, Query *query);
DocumentList *inv_index_document_search(DocumentList *all_docs, Query *query,
                                        InvertedIndex *index);

// Funcions auxiliars i d'utilitat
bool is_doc_already_in_list(DocumentList *list, int id);

#endif // QUERY_H
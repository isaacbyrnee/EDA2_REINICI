#ifndef QUERY_H
#define QUERY_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WORD_LENGTH 100

#include "documents.h"

typedef struct QueryItem {     // 1 paraula del search, ex: "hola que tal"
  char *word;                  //"hola"  "que"
  struct QueryItem *next_word; //"que"   "tal"
} QueryItem;

typedef struct Query {
  int size;              // Nombre de QueryItems (paraules) a la consulta
  QueryItem *first_word; // Punter al primer QueryItem de la llista
} Query;                 // Tot el search:

// ex: Query = "cute cats"
// QueryItem = "cute", "cats"

bool QueryItem_in_doc(Query *query, Document *doc);
DocumentList *document_search(DocumentList *docs, Query *query);
Query *InitQuery();
#endif // QUERY_H//

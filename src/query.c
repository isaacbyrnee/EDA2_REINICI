#include "query.h"
#include "documents.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

Query *InitQuery() {
  Query *query = (Query *)malloc(sizeof(Query));
  if (query == NULL)
    return NULL;
  query->first_word = NULL;
  query->size = 0;
  return query;
}

bool QueryItem_in_doc(Query *query, Document *doc) {
  if (query == NULL || doc == NULL) {
    return false;
  }
  // Establir com a QueryItem la paraula que cercarem
  QueryItem *current = query->first_word;
  int num_words = 0; // incicialitzar comptador

  while (current != NULL) { //*Mentre que no s'acabi la llista*
                            // 1. Comprovar si la paraula està en el títol
    if (strstr(doc->title, current->word) != NULL) {
      num_words++;
    }
    // 2. Comprovar si la paraula està en el body
    else if (strstr(doc->body, current->word) != NULL && doc->body != NULL) {
      num_words++;
    }
    current = current->next_word; // Passar a la següent paraula
  }
  if (query->size == num_words) { //*Si s'ha trobat totes les paraules o no*
    return true;
  } else {
    return false;
  }
}

// Funció per retornar llista amb documents que contenen tot el query (search)
DocumentList *document_search(DocumentList *docs, Query *query) {
  // ... (El codi de document_search que ja tenies, amb la còpia superficial)
  // ...
  DocumentList *docs_with_query = malloc(sizeof(DocumentList));
  if (docs_with_query == NULL) {
    return NULL;
  }
  docs_with_query->first_document = NULL;
  docs_with_query->size = 0;

  Document *current_doc = docs->first_document;
  Document *last_added_to_result_list = NULL;
  const int MAX_RESULTS_TO_FIND = 5;

  while (current_doc != NULL && docs_with_query->size < MAX_RESULTS_TO_FIND) {
    if (QueryItem_in_doc(query, current_doc) == true) {
      Document *new_result_node = (Document *)malloc(sizeof(Document));
      if (new_result_node == NULL) {
        // TODO: Free any already allocated nodes in docs_with_query before
        // returning NULL
        return NULL;
      }
      memcpy(new_result_node, current_doc, sizeof(Document));
      new_result_node->next_document = NULL;

      if (docs_with_query->first_document == NULL) {
        docs_with_query->first_document = new_result_node;
      } else {
        last_added_to_result_list->next_document = new_result_node;
      }
      last_added_to_result_list = new_result_node;
      docs_with_query->size++;
    }
    current_doc = current_doc->next_document;
  }
  return docs_with_query;
}
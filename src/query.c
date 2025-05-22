#include "query.h"
#include "documents.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

Query *InitQuery(char *search) {
  Query *query = (Query *)malloc(sizeof(Query));
  if (query == NULL) {
    return NULL; // Handle malloc failure for Query
  }

  // Allocate memory for the first QueryItem
  QueryItem *firstItem = (QueryItem *)malloc(sizeof(QueryItem));
  if (firstItem == NULL) {
    free(query); // Clean up previously allocated Query memory
    return NULL; // Handle malloc failure for QueryItem
  }

  // Assign the search word to the QueryItem.
  // IMPORTANT: Use strdup to make a copy of the string.
  // If you just assign 'search', 'firstItem->word' would point
  // to the local 'search' array in main, which goes out of scope.
  firstItem->word = strdup(search);
  if (firstItem->word == NULL) {
    free(firstItem);
    free(query);
    return NULL; // Handle strdup failure
  }
  firstItem->next_word =
      NULL; // Initialize next_word, as it's the only word for now

  query->first_word = firstItem; // Link the Query to its first QueryItem
  query->size = 1;

  return query;
}

bool QueryItem_in_doc(Query *query, Document *doc) {
  if (query == NULL || doc == NULL) {
    return false;
  }
  // Establir com a QueryItem la paraula que cercarem
  QueryItem *current = query->first_word;
  int num_words = 0; // incicialitzar comptador
  printf("la paraula current es:\n");

  while (current != NULL) { //*Mentre que no s'acabi la llista*
                            // 1. Comprovar si la paraula està en el títol

    if (strstr(doc->title, current->word) != NULL) {
      printf("Esta en el titol: %40s\n", doc->title);
      num_words++;
    }
    // 2. Comprovar si la paraula està en el body
    else if (strstr(doc->body, current->word) != NULL && doc->body != NULL) {
      printf("Esta en el body\n");
      num_words++;
    }
    current = current->next_word; // Passar a la següent paraula
  }
  if (query->size == num_words) { //*Si s'ha trobat totes les paraules o no*
    printf("Sí s'ha trobat les paraules al document %40s\n", doc->title);
    return true;
  } else {
    printf("No s'ha trobat les paraules al document %40s\n", doc->title);
    return false;
  }
}

// Funció per retornar llista amb documents que contenen tot el query (search)
DocumentList *document_search(DocumentList *docs, Query *query) {

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
    if (QueryItem_in_doc(query, current_doc)) {
      Document *new_result_node = (Document *)malloc(sizeof(Document));
      if (new_result_node == NULL) {
        return NULL;
      }

      new_result_node->id = current_doc->id;
      new_result_node->relevance = current_doc->relevance;

      // Copia profunda per al títol
      if (current_doc->title != NULL) {
        new_result_node->title =
            strdup(current_doc->title); // strdup: assigna memòria i copia
        if (new_result_node->title == NULL) {
          free(new_result_node);

          return NULL;
        }
      } else {
        new_result_node->title = NULL;
      }

      // Copia profunda per al body
      if (current_doc->body != NULL) {
        new_result_node->body =
            strdup(current_doc->body); // strdup: assigna memòria i copia
        if (new_result_node->body == NULL) {
          free(new_result_node->title); // Alliberar títol si ja s'havia copiat
          free(new_result_node);

          return NULL;
        }
      } else {
        new_result_node->body = NULL;
      }

      // Copia profunda per a la linklist
      if (current_doc->linklist != NULL) {
        LinkList *new_linklist =
            LinksInit(); // Inicialitzar una nova llista de links
        if (new_linklist == NULL) {
          free(new_result_node->title);
          free(new_result_node->body);
          free(new_result_node);

          return NULL;
        }
        Link *current_link = current_doc->linklist->first;
        while (current_link != NULL) {
          AddLink(new_linklist,
                  current_link->id); // Afegir cada link a la nova llista
          current_link = current_link->link_next;
        }
        new_result_node->linklist = new_linklist;
      } else {
        new_result_node->linklist = NULL;
      }

      new_result_node->next_document =
          NULL; // Molt important per a la nova llista enllaçada

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
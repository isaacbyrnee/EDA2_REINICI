#include "query.h"
#include "documents.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

Query *InitQuery(char *search) {
  Query *query = malloc(sizeof(Query));
  if (!query)
    return NULL;

  query->first_word = NULL;
  query->size = 0;
  QueryItem *last = NULL;

  // Separem la string per espais
  char *token = strtok(search, " ");
  while (token != NULL) {
    QueryItem *item = malloc(sizeof(QueryItem));
    if (!item)
      return NULL;

    item->word = strdup(token); // Pots fer servir `to_lowercase(token)` si vols
    printf("Paraula afegida al query: %s\n", token);
    item->next_word = NULL;

    if (last == NULL) {
      query->first_word = item;
    } else {
      last->next_word = item;
    }
    last = item;
    query->size++;

    token = strtok(NULL, " ");
  }

  return query;
}

bool QueryItem_in_doc(Query *query, Document *doc) {
  if (query == NULL || doc == NULL) {
    return false;
  }

  QueryItem *current = query->first_word;
  int num_words = 0;

  while (current != NULL) {
    bool found = false;

    // Comprovem al títol
    if (doc->title != NULL) {
      char *title_copy = strdup(doc->title);
      char *token = strtok(title_copy, " ,.!?;:\n");

      while (token != NULL) {
        if (strcasecmp(token, current->word) == 0) {
          found = true;
          printf("Paraula '%s' trobada al títol\n", current->word);
          break;
        }
        token = strtok(NULL, " ,.!?;:\n");
      }
      free(title_copy);
    }

    // Si no s’ha trobat al títol, busquem al body
    if (!found && doc->body != NULL) {
      char *body_copy = strdup(doc->body);
      char *token = strtok(body_copy, " ,.!?;:\n");

      while (token != NULL) {
        if (strcasecmp(token, current->word) == 0) {
          found = true;
          printf("Paraula '%s' trobada al cos\n", current->word);
          break;
        }
        token = strtok(NULL, " ,.!?;:\n");
      }
      free(body_copy);
    }

    if (found) {
      num_words++; // incrementem si s’ha trobat la paraula
    }

    current = current->next_word;
  }

  // OR: retornem true si almenys una paraula del query es troba al document
  if (num_words > 0) {
    printf("Almenys una paraula trobada al document: %s\n", doc->title);
    return true;
  } else {
    printf("Cap paraula trobada al document: %s\n", doc->title);
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
    if (!is_doc_already_in_list(docs_with_query, current_doc->id) &&
        QueryItem_in_doc(query, current_doc)) {

      Document *new_result_node = (Document *)malloc(sizeof(Document));
      if (new_result_node == NULL) {
        return NULL;
      }

      new_result_node->id = current_doc->id;
      new_result_node->relevance = current_doc->relevance;

      new_result_node->title =
          current_doc->title ? strdup(current_doc->title) : NULL;
      new_result_node->body =
          current_doc->body ? strdup(current_doc->body) : NULL;

      if (current_doc->linklist != NULL) {
        LinkList *new_linklist = LinksInit();
        if (!new_linklist) {
          free(new_result_node->title);
          free(new_result_node->body);
          free(new_result_node);
          return NULL;
        }
        Link *current_link = current_doc->linklist->first;
        while (current_link != NULL) {
          AddLink(new_linklist, current_link->id);
          current_link = current_link->link_next;
        }
        new_result_node->linklist = new_linklist;
      } else {
        new_result_node->linklist = NULL;
      }

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

  return docs_with_query; // ← ✅ Aquesta línia és molt important!
}

bool is_doc_already_in_list(DocumentList *list, int id) {
  Document *current = list->first_document;
  while (current != NULL) {
    if (current->id == id) {
      printf("⚠️ Ja hi és: ID %d\n", id);
      return true;
    }
    current = current->next_document;
  }
  return false;
}

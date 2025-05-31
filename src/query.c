// - - - query.c - - - //

#include "query.h"
#include "documents.h"
#include "inverted_index.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// --- GESTIÓ DE CONSULTES (QUERY) --- //

// Inicialitza i construeix una estructura Query a partir d'una cadena de text
// de cerca
Query *InitQuery(char *search) {
  Query *query = malloc(sizeof(Query));
  if (!query)
    return NULL;

  query->first_word = NULL;
  query->size = 0;
  QueryItem *last = NULL;

  char *token = strtok(search, COMMON_DELIMITERS);
  while (token != NULL) {
    QueryItem *item = malloc(sizeof(QueryItem));
    if (!item) {
      return NULL;
    }

    char *normalized_token = strdup(token);
    if (!normalized_token) {
      perror("strdup normalized_token failed");
      return NULL;
    }
    for (int i = 0; normalized_token[i]; i++) {
      normalized_token[i] = tolower((unsigned char)normalized_token[i]);
    }
    item->word = normalized_token; // Guardem la paraula normalitzada
    item->next_word = NULL;

    if (last == NULL) {
      query->first_word = item;
    } else {
      last->next_word = item;
    }
    last = item;
    query->size++;
    token = strtok(NULL, COMMON_DELIMITERS); // Usa COMMON_DELIMITERS aquí també
  }
  return query;
}

// Allibera tota la memòria assignada dinàmicament per a una Query i els seus
// elements interns
void free_query(Query *q) {
  if (q == NULL)
    return;
  QueryItem *current = q->first_word;
  while (current != NULL) {
    QueryItem *next = current->next_word;
    free(current->word); // Allibera la paraula
    free(current);       // Allibera el QueryItem
    current = next;
  }
  free(q); // Allibera l'estructura Query
}

// --- LÒGICA DE CERCA I FILTRATGE --- //

// Determina si un document donat conté totes les paraules d'una consulta
// específica
bool document_contains_all_query_words(Document *doc, Query *query) {
  if (query == NULL || doc == NULL || query->first_word == NULL) {
    return false; // Si no hi ha query o document, no pot contenir totes les
                  // paraules
  }

  QueryItem *current_query_word = query->first_word;

  while (current_query_word != NULL) {
    bool found_current_word_in_doc = false;

    // Les paraules de la query (current_query_word->word) ja estan en
    // minúscules per InitQuery.

    // Comprovem al títol
    if (doc->title != NULL) {
      char *title_copy = strdup(doc->title);
      if (!title_copy) {
        perror("strdup title_copy failed");
        return false;
      }

      char *token =
          strtok(title_copy, COMMON_DELIMITERS); // Usa COMMON_DELIMITERS
      while (token != NULL) {
        // Normalitzar el token del document a minúscules per comparar
        for (int i = 0; token[i]; i++) {
          token[i] = tolower((unsigned char)token[i]);
        }
        if (strcmp(token, current_query_word->word) ==
            0) { // Comparem amb strcmp
          found_current_word_in_doc = true;
          break;
        }
        token = strtok(NULL, COMMON_DELIMITERS); // Usa COMMON_DELIMITERS
      }
      free(title_copy);
    }

    // Si no s'ha trobat al títol, busquem al body
    if (!found_current_word_in_doc && doc->body != NULL) {
      char *body_copy = strdup(doc->body);
      if (!body_copy) {
        perror("strdup body_copy failed");
        return false;
      }

      char *token =
          strtok(body_copy, COMMON_DELIMITERS); // Usa COMMON_DELIMITERS
      while (token != NULL) {
        // Normalitzar el token del document a minúscules per comparar
        for (int i = 0; token[i]; i++) {
          token[i] = tolower((unsigned char)token[i]);
        }
        if (strcmp(token, current_query_word->word) ==
            0) { // Comparem amb strcmp
          found_current_word_in_doc = true;
          break;
        }
        token = strtok(NULL, COMMON_DELIMITERS); // Usa COMMON_DELIMITERS
      }
      free(body_copy);
    }

    // Lògica AND: Si la paraula actual NO es troba al document, retorna false
    // immediatament
    if (!found_current_word_in_doc) {

      return false;
    }

    current_query_word = current_query_word->next_word;
  }
  return true;
}

// Realitza una cerca de documents de forma lineal i retorna aquells que
// contenen totes les paraules de la consulta
DocumentList *linear_document_search(DocumentList *all_docs, Query *query) {
  DocumentList *results = malloc(sizeof(DocumentList));
  if (!results) {
    fprintf(
        stderr,
        "Error: Error en assignar memòria per a DocumentList de resultats.\n");
    return NULL;
  }
  results->first_document = NULL;
  results->size = 0;

  Document *current_doc_in_all_docs = all_docs->first_document;
  Document *last_added_to_results = NULL;

  while (current_doc_in_all_docs != NULL) {
    if (document_contains_all_query_words(current_doc_in_all_docs, query)) {
      // Crear una còpia del document per afegir a la llista de resultats
      Document *new_result_node = (Document *)malloc(sizeof(Document));
      if (new_result_node == NULL) {
        fprintf(stderr,
                "Error: Error en assignar memòria per al node de resultat.\n");
        free_document_list(results); // Alliberar el que ja s'ha afegit
        return NULL;
      }
      // Copiar les dades del document original (títols i cossos han de ser
      // strdup-ed)
      new_result_node->id = current_doc_in_all_docs->id;
      new_result_node->title = strdup(current_doc_in_all_docs->title);
      new_result_node->body = strdup(current_doc_in_all_docs->body);
      new_result_node->relevance = current_doc_in_all_docs->relevance;
      // Copiar la linklist (profundament)
      if (current_doc_in_all_docs->linklist) {
        LinkList *new_linklist = LinksInit();
        Link *original_link = current_doc_in_all_docs->linklist->first;
        while (original_link) {
          AddLink(new_linklist, original_link->id);
          original_link = original_link->link_next;
        }
        new_result_node->linklist = new_linklist;
      } else {
        new_result_node->linklist = NULL;
      }
      new_result_node->next_document = NULL;

      // Afegir a la llista de resultats
      if (results->first_document == NULL) {
        results->first_document = new_result_node;
      } else {
        last_added_to_results->next_document = new_result_node;
      }
      last_added_to_results = new_result_node;
      results->size++;
    }
    current_doc_in_all_docs = current_doc_in_all_docs->next_document;
  }

  // Ordenar els resultats per rellevància (descendent)
  documentsListSortedDescending(results);

  return results;
}

// Cerca de documents utilitzant l'índex invertit (ràpida, lògica AND)
DocumentList *inv_index_document_search(DocumentList *all_docs, Query *query,
                                        InvertedIndex *index) {
  DocumentList *results = malloc(sizeof(DocumentList));
  if (!results) {
    fprintf(
        stderr,
        "Error: Error en assignar memòria per a DocumentList de resultats.\n");
    return NULL;
  }
  results->first_document = NULL;
  results->size = 0;

  // Si la consulta està buida, no hi ha resultats
  if (query == NULL || query->first_word == NULL) {
    return results;
  }

  // Pas 1: Obtenir la llista d'IDs de documents per a la primera paraula de la
  // consulta
  QueryItem *first_query_word = query->first_word;
  DocEntry *initial_doc_ids = inverted_index_get(index, first_query_word->word);

  // Si la primera paraula no es troba, no hi ha resultats AND
  if (initial_doc_ids == NULL) {
    return results;
  }

  // Pas 2: Iterar sobre els IDs de documents de la primera paraula i verificar
  // les altres paraules
  DocEntry *current_doc_id_from_index = initial_doc_ids;
  Document *last_added_to_results = NULL;

  while (current_doc_id_from_index != NULL) {
    int doc_id = current_doc_id_from_index->doc_id;
    Document *candidate_doc =
        get_document_by_id(all_docs, doc_id); // Obtenir el document complet

    if (candidate_doc != NULL) {
      bool all_words_found = true;
      QueryItem *current_check_word =
          first_query_word->next_word; // Començar des de la segona paraula

      while (current_check_word != NULL) {
        DocEntry *word_docs =
            inverted_index_get(index, current_check_word->word);
        bool doc_id_found_for_this_word = false;
        DocEntry *temp_doc_entry = word_docs;

        while (temp_doc_entry != NULL) {
          if (temp_doc_entry->doc_id == doc_id) {
            doc_id_found_for_this_word = true;
            break;
          }
          temp_doc_entry = temp_doc_entry->next;
        }

        if (!doc_id_found_for_this_word) {
          all_words_found = false;
          break;
        }
        current_check_word = current_check_word->next_word;
      }

      if (all_words_found) {
        // Si totes les paraules es van trobar, afegir el document als resultats
        // Crear una còpia del document per afegir a la llista de resultats
        Document *new_result_node = (Document *)malloc(sizeof(Document));
        if (new_result_node == NULL) {
          fprintf(
              stderr,
              "Error: Error en assignar memòria per al node de resultat.\n");
          free_document_list(results); // Alliberar el que ja s'ha afegit
          return NULL;
        }
        // Copiar les dades del document original (títols i cossos han de ser
        // strdup-ed)
        new_result_node->id = candidate_doc->id;
        new_result_node->title = strdup(candidate_doc->title);
        new_result_node->body = strdup(candidate_doc->body);
        new_result_node->relevance = candidate_doc->relevance;
        // Copiar la linklist (profundament)
        if (candidate_doc->linklist) {
          LinkList *new_linklist = LinksInit();
          Link *original_link = candidate_doc->linklist->first;
          while (original_link) {
            AddLink(new_linklist, original_link->id);
            original_link = original_link->link_next;
          }
          new_result_node->linklist = new_linklist;
        } else {
          new_result_node->linklist = NULL;
        }
        new_result_node->next_document = NULL;

        // Afegir a la llista de resultats
        if (results->first_document == NULL) {
          results->first_document = new_result_node;
        } else {
          last_added_to_results->next_document = new_result_node;
        }
        last_added_to_results = new_result_node;
        results->size++;
      }
    }
    current_doc_id_from_index = current_doc_id_from_index->next;
  }
  // Pas 3: Ordenar els resultats per rellevància (descendent)
  documentsListSortedDescending(results);
  return results;
}

// --- FUNCIONS AUXILIARS I D'UTILITAT --- //

// Comprova si un document amb un ID específic ja existeix en una llista de
// documents donada
bool is_doc_already_in_list(DocumentList *list, int id) {
  Document *current = list->first_document;
  while (current != NULL) {
    if (current->id == id) {
      return true;
    }
    current = current->next_document;
  }
  return false;
}
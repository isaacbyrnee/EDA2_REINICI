#include "query.h"
#include "documents.h"
#include "inverted_index.h" // Asegúrate de incluir el nuevo header

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// ----- QUERY BASICS ----- //

Query *InitQuery(char *search) {
  Query *query = malloc(sizeof(Query));
  if (!query)
    return NULL;

  query->first_word = NULL;
  query->size = 0;
  QueryItem *last = NULL;

  // Separem la string per espais (y otros caracteres si quieres que el query
  // tenga los mismos delimitadores) Usamos COMMON_DELIMITERS para consistencia
  char *token = strtok(search, COMMON_DELIMITERS);
  while (token != NULL) {
    QueryItem *item = malloc(sizeof(QueryItem));
    if (!item) {
      // En un escenario real, deberías liberar los QueryItem ya asignados aquí.
      // Por simplicidad, por ahora solo retornamos NULL.
      return NULL;
    }

    // *** IMPORTANTE: NORMALIZAR LA PALABRA DE LA QUERY A MINÚSCULAS ***
    char *normalized_token = strdup(token);
    if (!normalized_token) {
      perror("strdup normalized_token failed");
      return NULL;
    }
    for (int i = 0; normalized_token[i]; i++) {
      normalized_token[i] = tolower((unsigned char)normalized_token[i]);
    }
    item->word = normalized_token; // Guardamos la palabra normalizada

    // printf("Paraula afegida al query (normalitzada): %s\n", item->word); //
    // Puedes comentar esto
    item->next_word = NULL;

    if (last == NULL) {
      query->first_word = item;
    } else {
      last->next_word = item;
    }
    last = item;
    query->size++;
    token =
        strtok(NULL, COMMON_DELIMITERS); // Usa COMMON_DELIMITERS aquí también
  }
  return query;
}
bool document_contains_all_query_words(Document *doc, Query *query) {
  if (query == NULL || doc == NULL || query->first_word == NULL) {
    return false; // Si no hay query o documento, no puede contener todas las
                  // palabras
  }

  QueryItem *current_query_word = query->first_word;

  while (current_query_word != NULL) {
    bool found_current_word_in_doc = false;

    // Las palabras de la query (current_query_word->word) ya están en
    // minúsculas por InitQuery.

    // Comprobamos en el título
    if (doc->title != NULL) {
      char *title_copy = strdup(doc->title);
      if (!title_copy) {
        perror("strdup title_copy failed");
        return false;
      }

      char *token =
          strtok(title_copy, COMMON_DELIMITERS); // Usa COMMON_DELIMITERS
      while (token != NULL) {
        // Normalizar el token del documento a minúsculas para comparar
        for (int i = 0; token[i]; i++) {
          token[i] = tolower((unsigned char)token[i]);
        }
        if (strcmp(token, current_query_word->word) ==
            0) { // Comparamos con strcmp
          found_current_word_in_doc = true;
          break;
        }
        token = strtok(NULL, COMMON_DELIMITERS); // Usa COMMON_DELIMITERS
      }
      free(title_copy);
    }

    // Si no se ha encontrado en el título, buscamos en el body
    if (!found_current_word_in_doc && doc->body != NULL) {
      char *body_copy = strdup(doc->body);
      if (!body_copy) {
        perror("strdup body_copy failed");
        return false;
      }

      char *token =
          strtok(body_copy, COMMON_DELIMITERS); // Usa COMMON_DELIMITERS
      while (token != NULL) {
        // Normalizar el token del documento a minúsculas para comparar
        for (int i = 0; token[i]; i++) {
          token[i] = tolower((unsigned char)token[i]);
        }
        if (strcmp(token, current_query_word->word) ==
            0) { // Comparamos con strcmp
          found_current_word_in_doc = true;
          break;
        }
        token = strtok(NULL, COMMON_DELIMITERS); // Usa COMMON_DELIMITERS
      }
      free(body_copy);
    }

    // *** LÓGICA AND: Si la palabra actual NO se encuentra en el documento,
    // retorna false inmediatamente. ***
    if (!found_current_word_in_doc) {
      // printf("No totes trobades al document: %s (Faltava: %s)\n", doc->title,
      // current_query_word->word); // Puedes dejar este para depurar
      return false;
    }

    current_query_word = current_query_word->next_word;
  }

  // Si llegamos aquí, significa que todas las palabras de la consulta se
  // encontraron en el documento. printf("Tot el query trobat al document:
  // %s\n", doc->title); // Puedes dejar este para depurar
  return true;
}

// Búsqueda lineal de documentos (AND)
DocumentList *linear_document_search(DocumentList *all_docs, Query *query) {
  DocumentList *results = malloc(sizeof(DocumentList));
  if (!results) {
    fprintf(
        stderr,
        "Error: Fallo al asignar memoria para DocumentList de resultados.\n");
    return NULL;
  }
  results->first_document = NULL;
  results->size = 0;

  Document *current_doc_in_all_docs = all_docs->first_document;
  Document *last_added_to_results = NULL;

  while (current_doc_in_all_docs != NULL) {
    if (document_contains_all_query_words(current_doc_in_all_docs, query)) {
      // Crear una copia del documento para añadir a la lista de resultados
      Document *new_result_node = (Document *)malloc(sizeof(Document));
      if (new_result_node == NULL) {
        fprintf(stderr,
                "Error: Fallo al asignar memoria para el nodo de resultado.\n");
        free_document_list(results); // Liberar lo que ya se ha añadido
        return NULL;
      }
      // Copiar los datos del documento original (títulos y cuerpos deben ser
      // strdup-ed)
      new_result_node->id = current_doc_in_all_docs->id;
      new_result_node->title = strdup(current_doc_in_all_docs->title);
      new_result_node->body = strdup(current_doc_in_all_docs->body);
      new_result_node->relevance = current_doc_in_all_docs->relevance;
      // Copiar la linklist (profundamente)
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

      // Añadir a la lista de resultados
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

  // Ordenar los resultados por relevancia (descendente)
  documentsListSortedDescending(results);

  return results;
}

// Búsqueda de documentos utilizando el índice invertido (rápida, lógica AND)
DocumentList *inv_index_document_search(DocumentList *all_docs, Query *query,
                                        InvertedIndex *index) {
  DocumentList *results = malloc(sizeof(DocumentList));
  if (!results) {
    fprintf(
        stderr,
        "Error: Fallo al asignar memoria para DocumentList de resultados.\n");
    return NULL;
  }
  results->first_document = NULL;
  results->size = 0;

  // Si la consulta está vacía, no hay resultados
  if (query == NULL || query->first_word == NULL) {
    return results;
  }

  // Paso 1: Obtener la lista de IDs de documentos para la primera palabra de la
  // consulta
  QueryItem *first_query_word = query->first_word;
  DocEntry *initial_doc_ids = inverted_index_get(index, first_query_word->word);

  // Si la primera palabra no se encuentra, no hay resultados AND
  if (initial_doc_ids == NULL) {
    return results;
  }

  // Paso 2: Iterar sobre los IDs de documentos de la primera palabra y
  // verificar las demás palabras
  DocEntry *current_doc_id_from_index = initial_doc_ids;
  Document *last_added_to_results = NULL;

  while (current_doc_id_from_index != NULL) {
    int doc_id = current_doc_id_from_index->doc_id;
    Document *candidate_doc =
        get_document_by_id(all_docs, doc_id); // Obtener el documento completo

    if (candidate_doc != NULL) {
      bool all_words_found = true;
      QueryItem *current_check_word =
          first_query_word->next_word; // Empezar desde la segunda palabra

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
        // Si todas las palabras se encontraron, añadir el documento a los
        // resultados Crear una copia del documento para añadir a la lista de
        // resultados
        Document *new_result_node = (Document *)malloc(sizeof(Document));
        if (new_result_node == NULL) {
          fprintf(
              stderr,
              "Error: Fallo al asignar memoria para el nodo de resultado.\n");
          free_document_list(results); // Liberar lo que ya se ha añadido
          return NULL;
        }
        // Copiar los datos del documento original (títulos y cuerpos deben ser
        // strdup-ed)
        new_result_node->id = candidate_doc->id;
        new_result_node->title = strdup(candidate_doc->title);
        new_result_node->body = strdup(candidate_doc->body);
        new_result_node->relevance = candidate_doc->relevance;
        // Copiar la linklist (profundamente)
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

        // Añadir a la lista de resultados
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
  // Paso 3: Ordenar los resultados por relevancia (descendente)
  documentsListSortedDescending(results);
  return results;
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

void free_query(Query *q) {
  if (q == NULL)
    return;
  QueryItem *current = q->first_word;
  while (current != NULL) {
    QueryItem *next = current->next_word;
    free(current->word); // Libera la palabra
    free(current);       // Libera el QueryItem
    current = next;
  }
  free(q); // Libera la estructura Query
}
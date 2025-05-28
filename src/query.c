#include "query.h"
#include "documents.h"
#include "inverted_index.h" // Asegúrate de incluir el nuevo header

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
// Nueva implementación de document_search usando el índice invertido
DocumentList *document_search(DocumentList *docs, Query *query,
                              InvertedIndex *index) {
  DocumentList *docs_with_query = malloc(sizeof(DocumentList));
  if (docs_with_query == NULL) {
    return NULL;
  }
  docs_with_query->first_document = NULL;
  docs_with_query->size = 0;

  Document *last_added_to_result_list = NULL;
  const int MAX_RESULTS_TO_FIND = 5;

  // Asumiremos una lógica de "OR" para las palabras de la consulta,
  // es decir, se buscan documentos que contengan al menos UNA de las palabras.
  // Si necesitas "AND" (todas las palabras), el algoritmo cambia.

  QueryItem *current_query_word = query->first_word;

  // Una lista temporal para almacenar los IDs de los documentos encontrados
  // Podemos usar una Linked List o un simple array dinámico si el número de
  // resultados esperados es bajo Por simplicidad, usaremos un enfoque que itera
  // las listas de resultados y las va añadiendo pero con cuidado de no añadir
  // duplicados.

  // Una forma simple de manejar los resultados de OR:
  // Iterar por cada palabra de la consulta.
  // Para cada palabra, obtener su lista de documentos del índice.
  // Añadir los documentos de esa lista al resultado final, evitando duplicados.

  // Para evitar duplicados eficientemente al construir la lista de resultados:
  // Podríamos usar un pequeño hash set temporal o ordenar las listas de
  // DocEntry por ID y luego hacer una unión. Para esta práctica, una simple
  // verificación de `is_doc_already_in_list` (aunque $O(N)$) es suficiente ya
  // que MAX_RESULTS_TO_FIND es pequeño.

  // También se puede crear una lista de "DocEntry" temporales y luego ordenarla
  // por ID y eliminar duplicados antes de construir la DocumentList final para
  // mejorar la eficiencia de la deduplicación si MAX_RESULTS_TO_FIND fuera muy
  // grande.

  // Para el requisito con 1 asterisco, una implementación sencilla es
  // suficiente:
  while (current_query_word != NULL &&
         docs_with_query->size < MAX_RESULTS_TO_FIND) {
    // Obtener la lista de documentos para esta palabra del índice
    DocEntry *word_docs = inverted_index_get(index, current_query_word->word);

    DocEntry *current_doc_id_in_index_list = word_docs;
    while (current_doc_id_in_index_list != NULL &&
           docs_with_query->size < MAX_RESULTS_TO_FIND) {
      int doc_id = current_doc_id_in_index_list->doc_id;

      // Si el documento no ha sido añadido ya a nuestra lista de resultados
      if (!is_doc_already_in_list(docs_with_query, doc_id)) {
        // Obtener la información completa del documento por su ID
        Document *original_doc = get_document_by_id(docs, doc_id);

        if (original_doc != NULL) {
          // Crear un nuevo nodo Document para la lista de resultados
          Document *new_result_node = (Document *)malloc(sizeof(Document));
          if (new_result_node == NULL) {
            // Manejo de error
            return NULL;
          }
          // Copiar los datos (title y body necesitan ser strdup-ed)
          new_result_node->id = original_doc->id;
          new_result_node->relevance =
              original_doc->relevance; // Inicialmente 0, se calculará más tarde

          new_result_node->title =
              original_doc->title ? strdup(original_doc->title) : NULL;
          new_result_node->body =
              original_doc->body ? strdup(original_doc->body) : NULL;

          // Copiar LinkList si es necesario (el requisito no lo pide para la
          // búsqueda, pero si se va a imprimir el documento completo, sí). Para
          // simplificar el lab 1, podemos dejar el linklist como NULL o
          // copiarlo si se va a usar en print_one_document desde searched_docs.
          // Para la práctica, asumiremos que se va a usar, aunque sea una copia
          // profunda.
          if (original_doc->linklist != NULL) {
            LinkList *new_linklist = LinksInit();
            if (!new_linklist) { /* handle error */
            }
            Link *current_link = original_doc->linklist->first;
            while (current_link != NULL) {
              AddLink(new_linklist, current_link->id);
              current_link = current_link->link_next;
            }
            new_result_node->linklist = new_linklist;
          } else {
            new_result_node->linklist = NULL;
          }
          new_result_node->next_document = NULL;

          // Añadir el nuevo nodo a la lista de resultados
          if (docs_with_query->first_document == NULL) {
            docs_with_query->first_document = new_result_node;
          } else {
            last_added_to_result_list->next_document = new_result_node;
          }
          last_added_to_result_list = new_result_node;
          docs_with_query->size++;
        }
      }
      current_doc_id_in_index_list = current_doc_id_in_index_list->next;
    }
    current_query_word = current_query_word->next_word;
  }
  return docs_with_query;
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

Document *get_document_by_id(DocumentList *list, int id) {
  Document *current = list->first_document;
  while (current != NULL) {
    if (current->id == id) {
      return current;
    }
    current = current->next_document;
  }
  return NULL;
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
// ----- En un nuevo archivo, por ejemplo, `inverted_index.c` -----
#include "inverted_index.h"
#include <stdio.h> // Para printf en debugging

// Función hash simple (djb2 by Dan Bernstein)
unsigned long hash_function(const char *str) {
  unsigned long hash = 5381;
  int c;
  while ((c = *str++))
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  return hash;
}

// Inicializa el índice invertido
InvertedIndex *inverted_index_init(int num_buckets) {
  InvertedIndex *index = (InvertedIndex *)malloc(sizeof(InvertedIndex));
  if (!index)
    return NULL;

  index->num_buckets = num_buckets;
  index->num_words = 0;
  index->buckets = (HashEntry **)calloc(
      num_buckets, sizeof(HashEntry *)); // calloc inicializa a NULL
  if (!index->buckets) {
    free(index);
    return NULL;
  }
  return index;
}

// Añade un par (palabra, doc_id) al índice
void inverted_index_add(InvertedIndex *index, const char *word, int doc_id) {
  if (!index || !word)
    return;

  unsigned long bucket_idx = hash_function(word) % index->num_buckets;

  // Buscar si la palabra ya existe en este cubo
  HashEntry *current_entry = index->buckets[bucket_idx];
  while (current_entry != NULL) {
    if (strcmp(current_entry->word, word) == 0) {
      // La palabra ya existe, ahora añadir el doc_id a su lista de documentos
      DocEntry *current_doc_entry = current_entry->doc_list;
      // Evitar duplicados: Si el doc_id ya está en la lista de esta palabra, no
      // lo añadimos de nuevo
      while (current_doc_entry != NULL) {
        if (current_doc_entry->doc_id == doc_id) {
          return; // Ya existe, no hacer nada
        }
        current_doc_entry = current_doc_entry->next;
      }
      // Si no se encontró el doc_id, añadirlo al principio de la lista
      DocEntry *new_doc_entry = (DocEntry *)malloc(sizeof(DocEntry));
      if (!new_doc_entry)
        return;
      new_doc_entry->doc_id = doc_id;
      new_doc_entry->next = current_entry->doc_list;
      current_entry->doc_list = new_doc_entry;
      return;
    }
    current_entry = current_entry->next;
  }

  // Si la palabra no existe en este cubo, crear una nueva HashEntry
  HashEntry *new_hash_entry = (HashEntry *)malloc(sizeof(HashEntry));
  if (!new_hash_entry)
    return;

  new_hash_entry->word = strdup(word); // Copia la palabra
  if (!new_hash_entry->word) {
    free(new_hash_entry);
    return;
  }

  // Crear la primera entrada de documento para esta palabra
  DocEntry *new_doc_entry = (DocEntry *)malloc(sizeof(DocEntry));
  if (!new_doc_entry) {
    free(new_hash_entry->word);
    free(new_hash_entry);
    return;
  }
  new_doc_entry->doc_id = doc_id;
  new_doc_entry->next = NULL;

  new_hash_entry->doc_list = new_doc_entry;
  new_hash_entry->next =
      index->buckets[bucket_idx]; // Añadir al principio del cubo
  index->buckets[bucket_idx] = new_hash_entry;
  index->num_words++;
}

// Busca una palabra en el índice
DocEntry *inverted_index_get(InvertedIndex *index, const char *word) {
  if (!index || !word)
    return NULL;

  unsigned long bucket_idx = hash_function(word) % index->num_buckets;
  HashEntry *current_entry = index->buckets[bucket_idx];

  while (current_entry != NULL) {
    if (strcmp(current_entry->word, word) == 0) {
      return current_entry
          ->doc_list; // Encontró la palabra, devuelve su lista de documentos
    }
    current_entry = current_entry->next;
  }
  return NULL; // Palabra no encontrada
}

// Libera la memoria del índice invertido
void inverted_index_free(InvertedIndex *index) {
  if (!index)
    return;

  for (int i = 0; i < index->num_buckets; i++) {
    HashEntry *current_hash_entry = index->buckets[i];
    while (current_hash_entry != NULL) {
      HashEntry *next_hash_entry = current_hash_entry->next;
      free(current_hash_entry->word); // Libera la palabra
      DocEntry *current_doc_entry = current_hash_entry->doc_list;
      while (current_doc_entry != NULL) {
        DocEntry *next_doc_entry = current_doc_entry->next;
        free(current_doc_entry); // Libera cada entrada de documento
        current_doc_entry = next_doc_entry;
      }
      free(current_hash_entry); // Libera la entrada de la tabla hash
      current_hash_entry = next_hash_entry;
    }
  }
  free(index->buckets); // Libera el array de buckets
  free(index);          // Libera la estructura del índice
}
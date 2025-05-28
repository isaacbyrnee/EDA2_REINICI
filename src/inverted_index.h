// ----- En `inverted_index.h` -----
#ifndef INVERTED_INDEX_H
#define INVERTED_INDEX_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// No necesitamos incluir "documents.h" aquí si DocEntry solo usa int doc_id.
// Si alguna función en este .h necesitara un "Document *" completo, haríamos un
// forward declaration. Por ahora, asumimos que no es necesario.

// Un "posting" o "entrada" para cada documento que contiene una palabra.
typedef struct DocEntry {
  int doc_id;
  struct DocEntry *next;
} DocEntry;

// Cada "cubo" (bucket) de la tabla hash contendrá una lista de estas entradas.
typedef struct HashEntry {
  char *word;
  DocEntry *doc_list;
  struct HashEntry *next;
} HashEntry;

// La estructura principal del Hashmap/Inverted Index
typedef struct InvertedIndex {
  HashEntry **buckets;
  int num_buckets;
  int num_words;
} InvertedIndex;

// --- Funciones para el Hashmap ---
InvertedIndex *inverted_index_init(int num_buckets);
unsigned long hash_function(const char *str);
void inverted_index_add(InvertedIndex *index, const char *word, int doc_id);
DocEntry *inverted_index_get(InvertedIndex *index, const char *word);
void inverted_index_free(InvertedIndex *index);

#endif // INVERTED_INDEX_H
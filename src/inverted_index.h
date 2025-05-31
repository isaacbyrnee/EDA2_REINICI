// - - - inverted_index.h - - - //

#ifndef INVERTED_INDEX_H
#define INVERTED_INDEX_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// --- ESTRUCTURES DE DADES DE L´'INDEX INVERTIT --- //

// Una "entrada" per cada document que conté una paraula.
typedef struct DocEntry {
  int doc_id;
  struct DocEntry *next;
} DocEntry;

// Entrada de la taula hash per a una paraula. Conté la paraula i la llista de
// documents associats.
typedef struct HashEntry {
  char *word;
  DocEntry *doc_list;
  struct HashEntry *next;
} HashEntry;

// Estructura principal de l'Índex Invertit (taula hash amb encadenament).
typedef struct InvertedIndex {
  HashEntry **buckets;
  int num_buckets;
  int num_words;
} InvertedIndex;

// --- DECLARACIONS DE FUNCIONS DE L'ÍNDEX INVERTIT ---

// Gestió de l'Índex Invertit
InvertedIndex *inverted_index_init(int num_buckets);
void inverted_index_add(InvertedIndex *index, const char *word, int doc_id);
DocEntry *inverted_index_get(InvertedIndex *index, const char *word);

// Calcular el valor hash d'una paraula.
unsigned long hash_function(const char *str);

// Free de l'índex invertit.
void inverted_index_free(InvertedIndex *index);

#endif // INVERTED_INDEX_H
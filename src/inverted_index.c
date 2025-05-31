// - - - inverted_index.c - - - //

#include "inverted_index.h"
#include <stdio.h>

// --- FUNCIÓ HASH: djb2 by Dan Bernstein --- //
unsigned long hash_function(const char *str) {
  unsigned long hash = 5381; // 1. Inicialització
  int c; // 2. Variable emmagatzemarà el valor ASCII de cada caràcter de la
         // string

  while ((c = *str++)) // 3. Bucle per cada caràcter de la string
    hash = ((hash << 5) + hash) + c;
  /* hash * 33 + c */ // 4. El cor de l'algorisme
  return hash;        // 5. Retorn del hash final
}

// --- GESTIÓ DE L'ÍNDEX INVERTIT --- //
// Funcions principals per a la inicialització, addició i consulta de l'índex.

// Inicialitza la taula hash de l'índex invertit
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

// Afegeix un parell (paraula, ID de document) a l'índex
void inverted_index_add(InvertedIndex *index, const char *word, int doc_id) {
  if (!index || !word)
    return;

  unsigned long bucket_idx = hash_function(word) % index->num_buckets;

  // Recorre la llista enllaçada del bucket per trobar la paraula (si ja
  // existeix)
  HashEntry *current_entry = index->buckets[bucket_idx];
  while (current_entry != NULL) {
    if (strcmp(current_entry->word, word) == 0) {
      // La paraula ja existeix, ara gestionem la llista d'ID de documents
      DocEntry *current_doc_entry = current_entry->doc_list;
      // Evita duplicats: si el doc_id ja està en la llista, no l'afegim de nou
      while (current_doc_entry != NULL) {
        if (current_doc_entry->doc_id == doc_id) {
          return; // El doc_id ja existeix per a aquesta paraula
        }
        current_doc_entry = current_doc_entry->next;
      }
      // Si el doc_id no es va trobar, l'afegim al principi de la llista de
      // documents
      DocEntry *new_doc_entry = (DocEntry *)malloc(sizeof(DocEntry));
      if (!new_doc_entry)
        return;
      new_doc_entry->doc_id = doc_id;
      new_doc_entry->next = current_entry->doc_list;
      current_entry->doc_list = new_doc_entry;
      return;
    }
    current_entry =
        current_entry->next; // Passa a la següent HashEntry en cas de col·lisió
  }

  // Si la paraula no existeix en aquest bucket, creem una nova HashEntry
  HashEntry *new_hash_entry = (HashEntry *)malloc(sizeof(HashEntry));
  if (!new_hash_entry)
    return;

  new_hash_entry->word = strdup(word); // Copia la paraula a una nova ubicació
  if (!new_hash_entry->word) {
    free(new_hash_entry);
    return;
  }

  // Creem la primera entrada de document per a aquesta nova paraula
  DocEntry *new_doc_entry = (DocEntry *)malloc(sizeof(DocEntry));
  if (!new_doc_entry) {
    free(new_hash_entry->word);
    free(new_hash_entry);
    return;
  }
  new_doc_entry->doc_id = doc_id;
  new_doc_entry->next = NULL;
  new_hash_entry->doc_list =
      new_doc_entry; // Afegim la nova HashEntry al principi de la llista
                     // enllaçada d'aquest bucket
  new_hash_entry->next = index->buckets[bucket_idx];
  index->buckets[bucket_idx] = new_hash_entry;
  index->num_words++;
}

// Busca una paraula a l'índex i retorna la seva llista d'ID de documents
DocEntry *inverted_index_get(InvertedIndex *index, const char *word) {
  if (!index || !word)
    return NULL;

  unsigned long bucket_idx = hash_function(word) % index->num_buckets;
  // Recorre la llista enllaçada del bucket corresponent
  HashEntry *current_entry = index->buckets[bucket_idx];

  while (current_entry != NULL) {
    if (strcmp(current_entry->word, word) == 0) {
      // Troba la paraula, retorna la seva llista de documents
      return current_entry->doc_list;
    }
    current_entry = current_entry->next; // Passa a la següent HashEntry
  }
  return NULL; // Paraula no trobada a l'índex
}

// --- ALLIBERAMENT DE MEMÒRIA (FREE) --- //
// Allibera la memòria de l'índex invertit complet
void inverted_index_free(InvertedIndex *index) {
  if (!index)
    return;

  for (int i = 0; i < index->num_buckets; i++) {
    HashEntry *current_hash_entry = index->buckets[i];
    while (current_hash_entry != NULL) {
      HashEntry *next_hash_entry = current_hash_entry->next;
      free(current_hash_entry->word); // Allibera la paraula (string)
      DocEntry *current_doc_entry = current_hash_entry->doc_list;
      while (current_doc_entry != NULL) {
        DocEntry *next_doc_entry = current_doc_entry->next;
        free(current_doc_entry); // Allibera cada entrada de document
        current_doc_entry = next_doc_entry;
      }
      free(current_hash_entry); // Allibera l'estructura HashEntry
      current_hash_entry = next_hash_entry;
    }
  }
  free(index->buckets); // Allibera l'array de buckets
  free(index);          // Allibera l'estructura InvertedIndex en si
}
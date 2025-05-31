// - - - documents.h - - - //

#ifndef DOCUMENTS_H
#define DOCUMENTS_H

// --- CONSTANTS --- //
#define MAX_LINE_LENGTH 10240 // Pots ajustar-ho si tens línies més llargues
#define MAX_BODY_LENGTH                                                        \
  100000 // Pots ajustar-ho si tens documents amb cosos més grans

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Query Query;
typedef struct InvertedIndex InvertedIndex;
typedef struct DocumentGraph DocumentGraph;

// Link structure
typedef struct Link {
  int id;
  struct Link *link_next;
} Link;

// Linked list of Links
typedef struct {
  int size;
  Link *first;
} LinkList;

// Document structure
typedef struct Document {
  int id;
  char *title;
  char *body;
  LinkList *linklist;
  float relevance;
  struct Document *next_document;
} Document;

// DocumentList structure
typedef struct {
  int size;
  Document *first_document;
} DocumentList;

// --- DECLARACIÓ DE FUNCIONS DELS DOCUMENTS --- //

// Funcions d'inicialització i per alliberar memòria
LinkList *LinksInit();
void LinksFree(LinkList *list);

// Funcions de gestió de documents
Document *document_desserialize(const char *path);
void AddLink(LinkList *linklist, int linkId);
Document *get_document_by_id(DocumentList *list, int id);

// Càrrega i processament general de documents
DocumentList *load_documents(char *half_path, int num_docs,
                             InvertedIndex *index, DocumentGraph *graph);

// Indexació i processament de text
void process_text_for_indexing(InvertedIndex *index, const char *text,
                               int doc_id);

// Impressió i visualització
void print_documents(DocumentList *docs, int max_to_print);
void print_one_document(int idx, DocumentList *list);

// Ordenació i alliberament de memòria
int compareDocuments(const void *a, const void *b);
DocumentList *documentsListSortedDescending(DocumentList *list);
void free_document_list_nodes(Document *doc_node);
void free_document_list(DocumentList *list);

#endif // DOCUMENTS_H
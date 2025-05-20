#ifndef DOCUMENTS_H // Esto es una guarda de inclusión
#define DOCUMENTS_H // El nombre suele ser el nombre del archivo en mayúsculas,
                    // seguido de _H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Lab 1
//  Link structure
typedef struct Link {
  int id;
  struct Link *link_next;
} Link;

// Linked list
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

typedef struct {
  int size;
  Document *first_document;
} DocumentList;

LinkList *LinksInit();
Document *document_desserialize(char *path);
void AddLink(LinkList *linklist, int linkId);
DocumentList *load_documents(char *half_path, int num_docs);
void print_all_documents(DocumentList *docs);
void print_one_document(int idx, DocumentList *list);
void free_link_list(LinkList *list);
void free_document(Document *doc);
void free_document_list(DocumentList *list);

#endif // DOCUMENTS_H//
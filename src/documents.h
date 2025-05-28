#ifndef DOCUMENTS_H
#define DOCUMENTS_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations para tipos que se usarán en las firmas de funciones
// pero cuyas definiciones completas están en otros archivos que pueden tener
// dependencias recíprocas.
typedef struct Query Query;
typedef struct InvertedIndex InvertedIndex; // <--- ¡AÑADIR ESTA LÍNEA!

// Lab 1
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

typedef struct {
  int size;
  Document *first_document;
} DocumentList;

// --- Funciones para LinkList ---
LinkList *LinksInit();
void AddLink(LinkList *linklist, int linkId);
void LinksFree(LinkList *list); // <--- ¡AÑADIR ESTA DECLARACIÓN!

// --- Funciones para Document ---
Document *document_desserialize(char *path);
// Ahora InvertedIndex es un tipo conocido gracias a la forward declaration
void process_text_for_indexing(InvertedIndex *index, const char *text,
                               int doc_id);
Document *get_document_by_id(DocumentList *list, int id); // Declarada aquí

// --- Funciones para DocumentList ---
// Firma actualizada para load_documents
DocumentList *load_documents(char *half_path, int num_docs,
                             InvertedIndex *index);
void print_all_documents(DocumentList *docs);
void print_one_document(int idx, DocumentList *list);
// Función para liberar la memoria de DocumentList
void free_document_list(DocumentList *list);
void free_document_list_nodes(Document *doc_node);

// La declaración de document_search ya no va aquí, va en query.h

#endif // DOCUMENTS_H
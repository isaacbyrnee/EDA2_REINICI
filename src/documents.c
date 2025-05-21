#include "documents.h"
#include "query.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LinkList *LinksInit() {
  LinkList *list = (LinkList *)malloc(sizeof(LinkList));

  if (list == NULL)
    return NULL;

  list->first = NULL;
  list->size = 0;
  return list;
}

Document *document_desserialize(char *path) {
  FILE *f = fopen(path, "r");
  assert(f != NULL);

  Document *document = (Document *)malloc(sizeof(Document));

  char buffer[262144];
  int bufferSize = 262144;
  int bufferIdx = 0;
  char ch;

  // parse id
  while ((ch = fgetc(f)) != '\n') {
    assert(bufferIdx < bufferSize);
    buffer[bufferIdx++] = ch;
  }
  assert(bufferIdx < bufferSize);
  buffer[bufferIdx++] = '\0';
  document->id = atoi(buffer);

  // parse title
  bufferIdx = 0;
  while ((ch = fgetc(f)) != '\n') {
    assert(bufferIdx < bufferSize);
    buffer[bufferIdx++] = ch;
  }
  assert(bufferIdx < bufferSize); // Asegurarse de que el buffer no se desbordó
  buffer[bufferIdx] = '\0';       // ¡Importante! Termina el string en buffer

  // Ahora, asigna memoria dinámicamente para el título del documento
  document->title = (char *)malloc(sizeof(char) * (strlen(buffer) + 1));
  if (!document->title) {
    // Manejo de error: liberar memoria ya asignada para 'document'
    // y devolver NULL o manejar la situación adecuadamente
    free(document);
    fclose(f);
    return NULL;
  }
  strcpy(document->title, buffer); // Copia el título al nuevo espacio asignado

  // parse body
  char linkBuffer[64];
  int linkBufferSize = 64;
  int linkBufferIdx = 0;
  bool parsingLink = false;
  LinkList *linklist = LinksInit();

  bufferIdx = 0;
  while ((ch = fgetc(f)) != EOF) {
    assert(bufferIdx < bufferSize);
    buffer[bufferIdx++] = ch;
    if (parsingLink) {
      if (ch == ')') { // end of link
        parsingLink = false;
        assert(linkBufferIdx < linkBufferSize);
        linkBuffer[linkBufferIdx++] = '\0';
        int linkId = atoi(linkBuffer);

        // TODO add to links
        AddLink(linklist, linkId);

        linkBufferIdx = 0;
      } else if (ch != '(') { // skip first parenthesis of the link
        assert(linkBufferIdx < linkBufferSize);
        linkBuffer[linkBufferIdx++] = ch;
      }
    } else if (ch ==
               ']') { // found beginning of link id, e.g.: [my link text](123)
      parsingLink = true;
    }
  }
  document->body = (char *)malloc(sizeof(char) * bufferIdx);
  assert(bufferIdx < bufferSize);
  buffer[bufferIdx++] = '\0';
  strcpy(document->body, buffer);

  fclose(f);

  return document;
}

// Funció per afegir links a la linklist
void AddLink(LinkList *linklist, int linkId) {
  Link *newlink = malloc(sizeof(Link)); // allocate memory
  if (!newlink)
    return;             // handle error allocation
  newlink->id = linkId; // set id of the link
  newlink->link_next =
      NULL; // as we want this to be the last element of our list

  if (linklist->first ==
      NULL) { // if the list is empty, set linklist.first to link.id
    linklist->first = newlink; // update the first element of the list
  } else { // if the list is not empty, find the last element of it
    Link *current = linklist->first; // access the first node to begin with
    while (current->link_next !=
           NULL) { // moves current until it finds the last node (which will be
                   // the element before the null terminator)
      current = current->link_next; // updates to the next element
    }

    // update the elements of the data structure
    current->link_next = newlink;
  }
  linklist->size += 1; // update the size of the list of links in any case
}

// LAB 1: Load documents from dataset/////////////////
DocumentList *
load_documents(char *half_path,
               int num_docs) { // half path = datasets/wikipediaXXX/
  // define and initialize document list
  DocumentList *list = (DocumentList *)malloc(sizeof(DocumentList));
  if (!list) {
    return NULL;
  }
  list->size = num_docs + 1;
  list->first_document = NULL;

  // define path for the first document
  char path[30];
  strcpy(path, half_path);
  strcat(path, "0.txt");

  // for the first document (we do this outside the loop since this way we can
  // store the info of the first doc in the variable first_document of the
  // DocumentList structure)
  Document *doc = document_desserialize(path);
  list->first_document = doc;

  char txt_str[12]; // where we'll store the number of the text as a string

  for (int txt = 1; txt <= num_docs;
       txt++) { // we start a loop where we'll iterate through each document and
                // add them to the list

    // prepare the path for the next document
    strcpy(path, half_path);
    sprintf(txt_str, "%d", txt);
    strcat(path, txt_str);
    strcat(path, ".txt");

    // desserialize the next document
    Document *next_doc = document_desserialize(path);

    // store in the document before, the info of the document we have just
    // 2desserialized
    doc->next_document = next_doc;

    // update the doc
    doc = next_doc;
  }

  return list;
}

// Lab 1: Print tots els documents
void print_all_documents(DocumentList *docs) {
  if (docs->first_document == NULL)
    return;
  Document *document = docs->first_document;

  int i = 0;
  while (document != NULL && i <= docs->size) {
    printf("(%d) %s\n", i, document->title);
    printf("---\n");

    int body_len = strlen(document->body);
    if (body_len >= 250) {
      printf("%.250s...\n", document->body);
    } else {
      printf("%s\n", document->body);
    }

    printf("---\n");
    printf("relevance score: %.0f\n\n\n", document->relevance);

    document = document->next_document;
    i++;
  }
}

// Lab1: Print 1 document amb la ID
void print_one_document(int idx, DocumentList *list) {
  Document *doc = list->first_document;
  if (list->first_document != NULL) {
    for (int i = 0; i < idx; i++) {
      doc = doc->next_document;
    }
  }
  if (doc == NULL)
    return;

  printf("\nID\n%d\n\n", doc->id);
  printf("TITLE\n%s\n\n", doc->title);
  printf("RELEVANCE SCORE:\n%0.f\n\n", doc->relevance);
  printf("BODY\n%s\n\n", doc->body);
  printf("-----------------------------\n");
}
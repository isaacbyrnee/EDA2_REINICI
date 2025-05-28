#include "documents.h"
#include "inverted_index.h"
#include "query.h"

#include <assert.h>
#include <ctype.h>
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
  if (f == NULL) {
    fprintf(stderr, "Error: No se pudo abrir el archivo %s\n", path);
    return NULL;
  }

  Document *document = (Document *)malloc(sizeof(Document));
  if (!document) {
    fprintf(stderr, "Error: Fallo al asignar memoria para Documento.\n");
    fclose(f);
    return NULL;
  }
  document->title = NULL;
  document->body = NULL;
  document->linklist = NULL;
  document->next_document = NULL;

  char buffer[262144];
  int bufferSize = sizeof(buffer);
  int bufferIdx = 0;
  char ch;

  // parse id
  while ((ch = fgetc(f)) != '\n' && ch != EOF) {
    if (bufferIdx >= bufferSize - 1) {
      fprintf(stderr, "Error: ID demasiado largo para el buffer en %s\n", path);
      free(document);
      fclose(f);
      return NULL;
    }
    buffer[bufferIdx++] = ch;
  }
  buffer[bufferIdx] = '\0';
  document->id = atoi(buffer);

  // parse title
  bufferIdx = 0;
  while ((ch = fgetc(f)) != '\n' && ch != EOF) {
    if (bufferIdx >= bufferSize - 1) {
      fprintf(stderr, "Error: Título demasiado largo para el buffer en %s\n",
              path);
      free(document);
      fclose(f);
      return NULL;
    }
    buffer[bufferIdx++] = ch;
  }
  buffer[bufferIdx] = '\0';

  document->title = (char *)malloc(strlen(buffer) + 1);
  if (!document->title) {
    fprintf(
        stderr,
        "Error: Fallo al asignar memoria para el título del documento %d.\n",
        document->id);
    free(document);
    fclose(f);
    return NULL;
  }
  strcpy(document->title, buffer);

  // parse body
  char linkBuffer[64];
  int linkBufferSize = sizeof(linkBuffer);
  int linkBufferIdx = 0;
  bool parsingLink = false;
  document->linklist = LinksInit();
  if (!document->linklist) {
    fprintf(stderr,
            "Error: Fallo al inicializar LinkList para el documento %d.\n",
            document->id);
    free(document->title);
    free(document);
    fclose(f);
    return NULL;
  }

  bufferIdx = 0;
  while ((ch = fgetc(f)) != EOF) {
    if (bufferIdx >= bufferSize - 1) {
      fprintf(stderr,
              "Error: Cuerpo del documento demasiado largo para el buffer en "
              "%s. Truncado.\n",
              path);
      break;
    }
    buffer[bufferIdx++] = ch; // Siempre añade el carácter al buffer del cuerpo

    // Lógica de parsing de enlaces
    if (parsingLink) {
      if (ch == ')') { // end of link ID
        parsingLink = false;
        if (linkBufferIdx >= linkBufferSize) {
          fprintf(stderr,
                  "Warning: ID de enlace demasiado largo en %s. Truncado.\n",
                  path);
        }
        linkBuffer[linkBufferIdx] = '\0';
        int linkId = atoi(linkBuffer);
        AddLink(document->linklist, linkId);
        linkBufferIdx = 0; // Reiniciar para el siguiente enlace
      } else {             // Caracteres del ID
        if (linkBufferIdx < linkBufferSize - 1) {
          linkBuffer[linkBufferIdx++] = ch;
        }
      }
    } else if (ch == '(') { // Inicia el parsing del ID de enlace
      // Asumo que el '(' es el inicio del ID numérico del enlace.
      // Si el formato es `[TEXTO](ID)`, entonces el '(' es el punto de inicio
      // del ID.
      parsingLink = true;
      linkBufferIdx = 0; // Reiniciar el linkBuffer
    }
  }

  if (bufferIdx < bufferSize) {
    buffer[bufferIdx] = '\0';
  } else {
    buffer[bufferSize - 1] = '\0';
  }

  document->body = (char *)malloc(strlen(buffer) + 1);
  if (!document->body) {
    fprintf(
        stderr,
        "Error: Fallo al asignar memoria para el cuerpo del documento %d.\n",
        document->id);
    free(document->title);
    LinksFree(document->linklist); // Usa free_document_list o LinksFree() si
                                   // tienes una para LinkList
    free(document);
    fclose(f);
    return NULL;
  }
  strcpy(document->body, buffer);

  fclose(f);
  return document;
}

void process_text_for_indexing(InvertedIndex *index, const char *text,
                               int doc_id) {
  if (!index || !text)
    return;

  char *text_copy = strdup(text);
  if (!text_copy)
    return;

  char *token = strtok(text_copy, " ,.!?;:[]()\n");
  while (token != NULL) {
    // --- NORMALIZAR LA PALABRA A MINÚSCULAS ANTES DE INDEXAR ---
    for (int i = 0; token[i]; i++) {
      token[i] =
          tolower((unsigned char)token[i]); // tolower espera un unsigned char
    }
    // --- FIN NORMALIZACIÓN ---
    inverted_index_add(index, token, doc_id);
    token = strtok(NULL, " ,.!?;:[]()\n");
  }
  free(text_copy);
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

// Modificación de load_documents para llenar el índice
DocumentList *load_documents(char *half_path, int num_docs,
                             InvertedIndex *index) {
  DocumentList *list = (DocumentList *)malloc(sizeof(DocumentList));
  if (!list) {
    return NULL;
  }
  list->size = num_docs + 1;
  list->first_document = NULL;

  char path[256]; // Aumenta el tamaño del buffer para el path si es necesario

  // Para el primer documento
  snprintf(path, sizeof(path), "%s0.txt",
           half_path); // Uso snprintf para seguridad
  Document *doc = document_desserialize(path);
  if (!doc) {
    free(list);
    return NULL;
  }
  list->first_document = doc;

  // Añadir título y body del primer documento al índice
  if (doc->title) {
    process_text_for_indexing(index, doc->title, doc->id);
  }
  if (doc->body) {
    process_text_for_indexing(index, doc->body, doc->id);
  }

  Document *current_doc_ptr =
      doc; // Usaremos un puntero para el encadenamiento de la lista

  for (int txt = 1; txt <= num_docs; txt++) {
    snprintf(path, sizeof(path), "%s%d.txt", half_path, txt);

    Document *next_doc = document_desserialize(path);
    if (!next_doc) {
      // Manejo de error si no se puede desserializar un documento
      // Podrías liberar la lista parcial y el índice o continuar si es
      // tolerable. Por simplicidad para la práctica, aquí podríamos saltar al
      // siguiente o salir.
      fprintf(stderr, "Error al desserializar documento %s. Saltando.\n", path);
      continue; // O break, dependiendo de la tolerancia a errores
    }

    current_doc_ptr->next_document =
        next_doc;               // Enlaza el documento anterior con el nuevo
    current_doc_ptr = next_doc; // Avanza el puntero

    // Añadir título y body del documento actual al índice
    if (next_doc->title) {
      process_text_for_indexing(index, next_doc->title, next_doc->id);
    }
    if (next_doc->body) {
      process_text_for_indexing(index, next_doc->body, next_doc->id);
    }
  }
  current_doc_ptr->next_document =
      NULL; // Asegurar que el último documento termina la lista

  return list;
}

// Lab 1: Print tots els documents
void print_all_documents(DocumentList *docs) {
  if (docs->first_document == NULL)
    return;
  Document *document = docs->first_document;

  int i = 0;
  while (document != NULL && i <= docs->size) {
    printf("ID: %d | TITOL: %s\n", document->id, document->title);
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

// Funció per alliberar la memòria d'una LinkList
void LinksFree(LinkList *list) {
  if (list == NULL)
    return;
  Link *current = list->first;
  while (current != NULL) {
    Link *next = current->link_next;
    free(current);
    current = next;
  }
  free(list); // Libera la estructura LinkList en sí
}

// ... (resto del archivo, incluyendo document_desserialize) ...
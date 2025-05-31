// - - - documents.c - - - //

#include "documents.h"
#include "document_graph.h"
#include "inverted_index.h"
#include "query.h"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- FUNCIONS D'INICIALITZACIÓ I PER ALLIBERAR MEMÒRIA --- //

// Inicialitza una nova llista d'enllaços
LinkList *LinksInit() {
  LinkList *list = (LinkList *)malloc(sizeof(LinkList));

  if (list == NULL)
    return NULL;

  list->first = NULL;
  list->size = 0;
  return list;
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
  free(list); // Allibera l'estructura LinkList en si
}

// --- FUNCIONS DE GESTIÓ DE DOCUMENTS --- //

// Desserialitza un document des d'un fitxer (llegeix i estructura les dades
// d'un document individual) Funció extreta i modificada del README.md

Document *document_desserialize(const char *path) {
  FILE *f = fopen(path, "r");
  if (!f) {
    perror("Error opening document file");
    return NULL;
  }

  Document *document = (Document *)malloc(sizeof(Document));
  if (!document) {
    fclose(f);
    return NULL;
  }

  document->id = -1; // Inicialitzem a un valor no vàlid
  document->title = NULL;
  document->body = NULL;
  document->relevance = 0; // S'actualitzarà després
  document->linklist = LinksInit();
  document->next_document = NULL;

  if (!document->linklist) {
    free(document);
    fclose(f);
    return NULL;
  }

  char line[MAX_LINE_LENGTH];             // Buffer per a cada línia llegida
  char temp_body_buffer[MAX_BODY_LENGTH]; // Per acumular el cos
  int bodyIdx = 0;

  bool parsingBody = false;
  bool parsingLink = false;
  char linkBuffer[20]; // Un buffer suficient per als IDs numèrics
  int linkBufferIdx = 0;
  bool expecting_link_id = false; // NOVA VARIABLE: per a ](

  while (fgets(line, sizeof(line), f) != NULL) {
    // Lectura de l'ID del document
    if (document->id == -1) {
      document->id = atoi(line);
      continue; // Passa a la següent línia
    }

    // Lectura del títol
    if (document->title == NULL) {
      line[strcspn(line, "\n")] = 0; // Elimina el salt de línia
      document->title = strdup(line);
      if (!document->title) {
        // Alliberar memòria i tancar fitxer en cas d'error
        LinksFree(document->linklist);
        free(document);
        fclose(f);
        return NULL;
      }
      // Un cop llegit el títol, comencem a parsejar el cos
      parsingBody = true;
      continue; // Passa a la següent línia
    }

    // Lectura del cos (i parsing d'enllaços alhora)
    if (parsingBody) {
      for (int i = 0; line[i] != '\0'; i++) {
        char ch = line[i];

        // Afegeix el caràcter al cos del document (important fer-ho abans de
        // la lògica d'enllaços per mantenir el text original)
        if (bodyIdx < MAX_BODY_LENGTH - 1) {
          temp_body_buffer[bodyIdx++] = ch;
        }

        // --- LÒGICA D'ENLLAÇOS MILLORADA: Detecta ]( i parseja l'ID numèric
        // ---
        if (expecting_link_id) {
          if (ch == '(') {
            // Hem trobat `](`, ara comencem a parsejar l'ID
            parsingLink = true;
            expecting_link_id = false; // Ja no esperem un '(', ja l'hem trobat
            linkBufferIdx = 0;         // Reiniciar el buffer per a l'ID
          } else {
            // Estàvem esperant '(', però no l'hem trobat immediatament.
            // Això no era un enllaç vàlid. Reiniciem l'estat.
            expecting_link_id = false;
          }
        } else if (parsingLink) {
          // Estem dins d'un enllaç [TEXTO](ID) -> parsejant l'ID
          if (ch == ')') {
            parsingLink = false;
            // Assegura que el buffer no desbordi i que el ID sigui numèric
            if (linkBufferIdx >= (int)sizeof(linkBuffer)) {
              fprintf(
                  stderr,
                  "Warning: ID de enlace demasiado largo en %s. Truncado.\n",
                  path);
            }
            linkBuffer[linkBufferIdx] = '\0';

            // Validar que el buffer conté només dígits abans de convertir
            bool is_numeric = true;
            if (linkBufferIdx == 0) { // Un ID buit () no és vàlid
              is_numeric = false;
            } else {
              for (int k = 0; k < linkBufferIdx; k++) {
                if (!isdigit((unsigned char)linkBuffer[k])) {
                  is_numeric = false;
                  break;
                }
              }
            }

            if (is_numeric) {
              int linkId = atoi(linkBuffer);
              AddLink(document->linklist, linkId);
            }
            linkBufferIdx = 0; // Reiniciar per al següent enllaç
          } else {
            // Encara llegint caràcters de l'ID
            if (linkBufferIdx < (int)sizeof(linkBuffer) - 1) {
              linkBuffer[linkBufferIdx++] = ch;
            }
          }
        } else if (ch == ']') {
          // Hem trobat un ']', ara esperem un '(' immediatament després
          expecting_link_id = true;
        }
        // --- FINAL DE LA LÒGICA D'ENLLAÇOS MILLORADA ---
      }
    }
  }

  // Finalitzar el cos del document
  temp_body_buffer[bodyIdx] = '\0';
  document->body = strdup(temp_body_buffer);
  if (!document->body) {
    LinksFree(document->linklist);
    free(document->title);
    free(document);
    fclose(f);
    return NULL;
  }

  fclose(f);
  return document;
}

// Funció per afegir links a la linklist
void AddLink(LinkList *linklist, int linkId) {
  Link *newlink = malloc(sizeof(Link)); // Assignar memòria
  if (!newlink)
    return;             // Gestionar l'assignació d'errors
  newlink->id = linkId; // Estableix l'identificador de l'enllaç
  newlink->link_next =
      NULL; // Com volem que aquest sigui l'últim element de la nostra llista

  if (linklist->first ==
      NULL) { // Si la llista està buida, estableix linklist.first a link.id
    linklist->first = newlink; // Actualitza el primer element de la llista
  } else { // Si la llista no està buida, trobeu-ne l'últim element
    Link *current = linklist->first; // Accedir al primer node per començar
    while (current->link_next !=
           NULL) { // Mou el corrent fins que troba l'últim node (que serà
                   // l'element anterior al terminador nul)
      current = current->link_next; // Actualitzacions al següent element
    }

    // Actualitzar els elements de l'estructura de dades
    current->link_next = newlink;
  }
  linklist->size +=
      1; // Actualitza la mida de la llista d'enllaços en qualsevol cas
}

// Cerca i retorna un document pel seu ID dins d'una llista de documents
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

// --- CÀRREGA I PROCESSAMENT GENERAL DE DOCUMENTS --- //

// Carrega múltiples documents, els afegeix a una llista, els indexa i els
// incorpora al graf de documents
DocumentList *load_documents(char *half_path, int num_docs,
                             InvertedIndex *index, DocumentGraph *graph) {
  DocumentList *list = (DocumentList *)malloc(sizeof(DocumentList));
  if (!list) {
    fprintf(stderr, "Error: Fallo al asignar memoria para DocumentList.\n");
    return NULL;
  }
  list->size = 0; // S'actualitzarà a mesura que s'aconseguiran documents
  list->first_document = NULL;

  char path[256];
  Document *current_doc_ptr = NULL;

  // Carregar documents i afegir nodos al graf
  for (int txt = 0; txt <= num_docs; txt++) {
    snprintf(path, sizeof(path), "%s%d.txt", half_path, txt);

    Document *new_doc = document_desserialize(path);
    if (!new_doc) {
      fprintf(stderr, "Error al desserializar documento %s. Saltando.\n", path);
      continue;
    }

    // Afegeix el document a la llista de documents
    if (list->first_document == NULL) {
      list->first_document = new_doc;
    } else {
      current_doc_ptr->next_document = new_doc;
    }
    current_doc_ptr = new_doc;
    list->size++;

    // Afegeix títol i cos del document a l'índex invertit
    if (new_doc->title) {
      process_text_for_indexing(index, new_doc->title, new_doc->id);
    }
    if (new_doc->body) {
      process_text_for_indexing(index, new_doc->body, new_doc->id);
    }

    // Afegeix el nodo del document al gràfic
    graph_add_node(graph, new_doc->id);
  }
  if (current_doc_ptr) {
    current_doc_ptr->next_document =
        NULL; // S'assegura que el darrer document acaba la llista
  }

  // Quan tots els nodes estan al graf, afegir les arestes
  Document *doc_iter = list->first_document;
  while (doc_iter != NULL) {
    Link *current_link = doc_iter->linklist->first;
    while (current_link != NULL) {
      if (graph_node_exists(graph, current_link->id)) {
        graph_add_edge(graph, doc_iter->id, current_link->id);
      }
      current_link = current_link->link_next;
    }
    doc_iter = doc_iter->next_document;
  }

  // Calcular i assignar la rellevància (indegree) a cada document
  doc_iter = list->first_document;
  while (doc_iter != NULL) {
    doc_iter->relevance = (float)graph_get_indegree(graph, doc_iter->id);
    doc_iter = doc_iter->next_document;
  }

  return list;
}

// --- INDEXACIÓ I PROCESSAMENT DE TEXT --- //

// Processa el text d'un document per afegir-lo a l'índex invertit amb la
// normalització a minúscules
void process_text_for_indexing(InvertedIndex *index, const char *text,
                               int doc_id) {
  if (!index || !text)
    return;

  char *text_copy = strdup(text);
  if (!text_copy)
    return;

  char *token = strtok(text_copy, COMMON_DELIMITERS);
  while (token != NULL) {
    // Normalitzar la paraula a minúscules abans d'indexar
    for (int i = 0; token[i]; i++) {
      token[i] =
          tolower((unsigned char)token[i]); // tolower espera un unsigned char
    }
    // Fi de la normalització
    inverted_index_add(index, token, doc_id);
    token = strtok(NULL, COMMON_DELIMITERS);
  }
  free(text_copy);
}

// --- IMPRESSIÓ I VISUALITZACIÓ --- //

// Imprimeix una llista de documents fins a un màxim especificat
void print_documents(DocumentList *docs, int max_to_print) {
  if (docs == NULL || docs->first_document == NULL)
    return;
  Document *document = docs->first_document;

  int i = 0;
  while (document != NULL && i < max_to_print) { // Canviat a 'i < max_to_print'
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

// Imprimeix els detalls d'un únic document donat el seu ID
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
  printf("TÍTOL\n%s\n\n", doc->title);
  printf("PUNUTACIÓ RELEVÀNCIA\n%0.f\n\n", doc->relevance);
  printf("COS\n%s\n\n", doc->body);
  printf("----------------------------------------------------------\n");
}

// --- ORDENACIÓ I ALLIBERAMENT DE MEMÒRIA --- //

// Funció de comparació utilitzada per qsort per ordenar documents per
// rellevància
int compareDocuments(const void *a, const void *b) {
  Document *docA = *(Document **)a;
  Document *docB = *(Document **)b;
  if (docA->relevance < docB->relevance)
    return 1;
  if (docA->relevance > docB->relevance)
    return -1;
  return 0;
}

// Ordena una llista de documents per la seva rellevància de forma descendent
DocumentList *documentsListSortedDescending(DocumentList *list) {
  if (list == NULL || list->first_document == NULL || list->size <= 1) {
    return list; // No hi ha res a ordenar o ja està ordenat
  }

  // Convertir la llista enllaçada a un array de punters a Documents per ordenar
  Document **doc_array = (Document **)malloc(list->size * sizeof(Document *));
  if (doc_array == NULL) {
    fprintf(stderr,
            "Error: Fallo al asignar memoria para el array de ordenación.\n");
    return NULL;
  }

  Document *current = list->first_document;
  for (int i = 0; i < list->size; i++) {
    doc_array[i] = current;
    current = current->next_document;
  }

  // Ordenar l'array usant qsort
  qsort(doc_array, list->size, sizeof(Document *), compareDocuments);

  // Reconstruir la llista enllaçada a partir de l'array ordenat
  list->first_document = doc_array[0];
  for (int i = 0; i < list->size - 1; i++) {
    doc_array[i]->next_document = doc_array[i + 1];
  }
  doc_array[list->size - 1]->next_document =
      NULL; // Assegurar el final de la llista

  free(doc_array); // Alliberar l'array temporal
  return list;
}

// Allibera recursivament la memòria dels nodes individuals de la llista de
// documents
void free_document_list_nodes(Document *doc_node) {
  if (doc_node == NULL)
    return;
  free_document_list_nodes(
      doc_node->next_document); // Alliberar recursivament els següents
  free(doc_node->title);        // Allibera memòria del títol
  free(doc_node->body);         // Allibera memòria del cos

  // Alliberar la linklist associada
  LinksFree(doc_node->linklist); // Fa servir la funció LinksFree per alliberar
                                 // la llista d'enllaços
  free(doc_node);                // Allibera el node del document
}

// Allibera la memòria de tota l'estructura DocumentList amb tots els seus nodes
void free_document_list(DocumentList *list) {
  if (list == NULL)
    return;
  free_document_list_nodes(list->first_document); // Allibera tots els nodes
  free(list); // Allibera l'estructura de la llista
}
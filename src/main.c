// - - - main.c - - - //

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "document_graph.h" // Conté les estructures i funcions del Graf de Documents
#include "documents.h"      // Conté les estructures i funcions de Documents
#include "inverted_index.h" // Conté les estructures i funcions de l'Índex Invertit
#include "query.h"          // Conté les estructures i funcions de Query
#include "queue.h" // Conté les estructures i funcions de la Cola d'historial

// --- FUNCIÓ NETEJA DE BUFFERS (SCANF) --- //

void clear_input_buffer() {
  int c;
  while ((c = getchar()) != '\n' && c != EOF)
    ;
}

// --- FUNCIÓ PRINCIPAL DEL PROGRAMA --- //
int main() {
  // --- Declaració de Variables ---
  char search_query_buffer[256]; // Buffer per a emmagatzemar la consulta de
                                 // l'usuari
  int wikipedia_choice;          // Elecció de la base de dades de Wikipedia
  int menu_choice;               // Elecció de l'opció del menú principal
  DocumentList
      *all_docs_list;  // Llista que contindrà tots els documents carregats
  Query *query = NULL; // Punter a l'estructura de la consulta actual

  // Variables per emmagatzemar la configuració de la Wikipedia triada
  char *selected_half_path = NULL; // Ruta parcial del directori dels documents
  int selected_num_docs = 0;       // Nombre de documents a carregar

  // Inicialitza la cua per a l'historial de cerques
  Queue *query_history = create_queue();
  if (!query_history) {
    fprintf(stderr, "Error al inicialitzar l'historial de búsquedes.\n");
    return 1; // Retorna 1 per indicar un error d'execució
  }

  // Inicialitza l'índex invertit
  InvertedIndex *global_inverted_index = inverted_index_init(1000);
  if (!global_inverted_index) {
    fprintf(stderr, "Error al inicialitzar l'índex invertit.\n");
    free_queue(query_history); // Allibera la cua abans de sortir
    return 1;
  }

  // Inicialitza el graf de documents
  DocumentGraph *document_graph = graph_init();
  if (!document_graph) {
    fprintf(stderr, "Error al inicialitzar el graf de documents.\n");
    inverted_index_free(global_inverted_index); // Allibera l'índex invertit
    free_queue(query_history);                  // Allibera la cua
    return 1;
  }

  // --- Bucle de Selecció de Base de Dades de Wikipedia ---
  while (1) {
    printf("\n           --- BENVINGUT A GOOGLE.UPF ---\n");
    printf("\n   --- SELECCIONA LA BASE DE DADES DE WIKIPEDIA ---\n\n");
    printf("               1. 0 - 12\n");
    printf("               2. 0 - 270\n");
    printf("               3. 0 - 540\n");
    printf("               4. 0 - 5400\n\n");
    printf("       * A cada base hi ha diferents documents *\n");
    printf("\nTria una opció: ");

    if (scanf("%d", &wikipedia_choice) != 1) {
      printf("Entrada no vàlida. Introdueix un número si us plau.\n");
      clear_input_buffer();
      continue;
    }
    clear_input_buffer();
    switch (wikipedia_choice) {
    case 1:
      selected_half_path = "datasets/wikipedia12/";
      selected_num_docs = 12;
      break;
    case 2:
      selected_half_path = "datasets/wikipedia270/";
      selected_num_docs = 270;
      break;
    case 3:
      selected_half_path = "datasets/wikipedia540/";
      selected_num_docs = 540;
      break;
    case 4:
      selected_half_path = "datasets/wikipedia5400/";
      selected_num_docs = 5400;
      break;
    default:
      printf("Opció invàida. Intenta-ho de nou.\n");
      continue; // Torna a mostrar el menú
    }
    break; // Surt del bucle un cop la selecció és vàlida
  }

  // --- CÀRREGA DE DOCUMENTS, CONSTRUCCIÓ D'ÍNDEX I GRAF ---
  all_docs_list = load_documents(selected_half_path, selected_num_docs,
                                 global_inverted_index, document_graph);

  // --- Bucle Principal del Menú del Motor de Cerca ---
  while (1) {
    print_queue(query_history); // Mostra l'historial de les últimes cerques
    printf("\n           --- MENÚ DEL MOTOR DE BÚSQUEDA ---\n\n");
    printf("1. Cerca Lineal\n");
    printf("2. Cerca d'Índex Invertit \n");
    printf("3. Top 5 Documents més Rellevants Globalment\n");
    printf("0. Sortir\n");
    printf("\nTria una opció: ");

    if (scanf("%d", &menu_choice) != 1) {
      printf("Entrada no vàlida. Si us plau, introdueix un número.\n");
      clear_input_buffer();
      continue;
    }
    clear_input_buffer();
    DocumentList *searched_docs_for_selection =
        NULL; // Llista temporal per als documents resultants de la cerca actual

    switch (menu_choice) {
    case 1: { // --- Opció: Cerca Lineal (Lògica AND) ---
      printf("\nCerca: ");
      if (fgets(search_query_buffer, sizeof(search_query_buffer), stdin) ==
          NULL) {
        menu_choice = 0; // Surt en cas d'error o EOF
        break;
      }
      search_query_buffer[strcspn(search_query_buffer, "\n")] =
          0; // Elimina el salt de línia

      if (strlen(search_query_buffer) == 0) {
        printf("Cerca buida. Tornant al menú.\n");
        continue; // Torna al menú si la cerca està buida
      }
      enqueue(query_history,
              search_query_buffer); // Afegeix la consulta a l'historial

      query = InitQuery(
          search_query_buffer); // Inicialitza l'estructura de la consulta
      if (!query) {
        printf("No s'ha pogut inicialitzar la query.\n");
        continue; // Torna al menú si la query falla
      }

      // Realitza la cerca lineal a través de tots els documents
      searched_docs_for_selection =
          linear_document_search(all_docs_list, query);
      printf("\n           --- DOCUMENTS TROBATS ---\n\n");
      if (searched_docs_for_selection &&
          searched_docs_for_selection->first_document) {
        print_documents(searched_docs_for_selection,
                        5); // Mostra els primers 5 documents trobats
      } else {
        printf("             Ningún document trobat.\n");
      }
      break;
    }

    case 2: { // --- Opció: Cerca amb Índex Invertit (Lògica AND i OR separada)
              // ---
      printf("\nCerca: ");
      if (fgets(search_query_buffer, sizeof(search_query_buffer), stdin) ==
          NULL) {
        menu_choice = 0; // Surt en cas d'error o EOF
        break;
      }
      search_query_buffer[strcspn(search_query_buffer, "\n")] =
          0; // Elimina el salt de línia

      if (strlen(search_query_buffer) == 0) {
        printf("Cerca buida. Tornant al menú.\n");
        continue; // Torna al menú si la cerca està buida
      }
      enqueue(query_history,
              search_query_buffer); // Afegeix la consulta a l'historial

      query = InitQuery(
          search_query_buffer); // Inicialitza l'estructura de la consulta
      if (!query) {             /* handle error */
        continue;               // Torna al menú si la query falla
      }

      // Crida a la funció de cerca amb índex invertit
      searched_docs_for_selection = inv_index_document_search(
          all_docs_list, query, global_inverted_index);
      printf("\n           --- DOCUMENTS TROBATS ---\n\n");
      if (searched_docs_for_selection &&
          searched_docs_for_selection->first_document) {
        // La funció `inv_index_document_search` ja ordena els documents per
        // rellevància.
        print_documents(searched_docs_for_selection,
                        5); // Mostra els primers 5 documents
      } else {
        printf("             Ningún document trobat.\n");
      }
      break;
    }

    case 3: { // --- Opció: Top 5 Documents per Rellevància Global ---
      printf("Mostrant els 5 documentos més rellevants globalment...\n");
      // Crea una còpia de la llista completa de documents per poder ordenar-la
      // sense modificar l'estructura original.
      DocumentList *temp_all_docs_copy = malloc(sizeof(DocumentList));
      if (!temp_all_docs_copy) {
        fprintf(
            stderr,
            "Error: Fallo al asignar memoria para copia de DocumentList.\n");
        continue;
      }
      temp_all_docs_copy->first_document = NULL;
      temp_all_docs_copy->size = 0;

      // Recorre la llista original i crea una còpia profunda dels documents.
      Document *current_original_doc = all_docs_list->first_document;
      Document *last_copy_node = NULL;
      while (current_original_doc != NULL) {
        Document *new_copy_node = (Document *)malloc(sizeof(Document));
        if (!new_copy_node) {
          fprintf(stderr,
                  "Error: Fallo al asignar memoria para nodo de copia.\n");
          free_document_list(
              temp_all_docs_copy); // Allibera la memòria ja copiada
          temp_all_docs_copy = NULL;
          break;
        }
        // Copia les dades del document original al nou node.
        new_copy_node->id = current_original_doc->id;
        new_copy_node->title = strdup(current_original_doc->title);
        new_copy_node->body = strdup(current_original_doc->body);
        new_copy_node->relevance = current_original_doc->relevance;
        new_copy_node->linklist =
            NULL; // No cal copiar la llista d'enllaços per a aquesta vista.
        new_copy_node->next_document = NULL;

        // Afegeix el nou node a la llista de còpia.
        if (temp_all_docs_copy->first_document == NULL) {
          temp_all_docs_copy->first_document = new_copy_node;
        } else {
          last_copy_node->next_document = new_copy_node;
        }
        last_copy_node = new_copy_node;
        temp_all_docs_copy->size++;
        current_original_doc = current_original_doc->next_document;
      }

      if (temp_all_docs_copy) {
        // Ordena la llista de còpia per rellevància global (descendent).
        searched_docs_for_selection =
            documentsListSortedDescending(temp_all_docs_copy);
        printf("\n           --- Documents Trobats ---\n");
        printf("\n   --- Top 5 Documents Globalment Més Rellevants ---\n\n");
        print_documents(searched_docs_for_selection,
                        5); // Mostra els primers 5.
      } else {
        printf("No s'ha pogut preparar la llista per mostrar la rellevància "
               "global.\n");
      }
      break;
    }

    case 0: // --- Opció: Sortir del Programa ---
      printf("\nSortint del programa...\n");
      break; // Surt del switch

    default:
      printf("Opció invàlida. Intenta-ho de nou.\n");
      continue; // Torna al principi del bucle principal
    }

    if (menu_choice == 0) {
      break; // Surt del bucle principal 'while' si l'usuari ha triat sortir
    }

    // --- Selecció de Document per a Visualització Detallada ---
    // Aquesta secció s'executa si s'han trobat documents en la cerca.
    if (searched_docs_for_selection == NULL ||
        searched_docs_for_selection->first_document == NULL) {
      // El missatge "Ningún document trobat" ja es gestiona dins de cada cas de
      // cerca. No necessitem un missatge genèric aquí.
    } else {
      printf("Tria l'ÍNDEX d'un Document (X) per veure més detalls (0-4, o -1 "
             "per fer una nova cerca): ");
      int doc_selected_idx;
      if (scanf("%d", &doc_selected_idx) != 1) {
        printf("Entrada no vàlida. Tornant al menú.\n");
        clear_input_buffer(); // Neteja el buffer
      } else {
        clear_input_buffer(); // Neteja el buffer d'entrada
        if (doc_selected_idx != -1) {
          print_one_document(
              doc_selected_idx,
              searched_docs_for_selection); // Mostra els detalls del document
                                            // seleccionat
        }
      }
    }

    // --- Alliberament de Memòria Temporal de la Cerca ---
    if (query) {
      free_query(query);
      query = NULL;
    }
    if (searched_docs_for_selection) {
      free_document_list(searched_docs_for_selection);
      searched_docs_for_selection = NULL;
    }
  }

  // --- Alliberament de Memòria Global en Sortir del Programa ---
  free_queue(query_history); // Allibera l'historial de cerques
  free_document_list(
      all_docs_list); // Allibera la llista completa de documents carregats
  inverted_index_free(global_inverted_index); // Allibera l'índex invertit
  graph_free(document_graph);                 // Allibera el graf de documents

  return 0;
}
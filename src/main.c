#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // Necesario para algunas operaciones de archivo, aunque no directamente usado en este snippet

#include "document_graph.h" // Contiene las estructuras y funciones del Grafo de Documentos
#include "documents.h" // Contiene las estructuras y funciones de Documentos
#include "inverted_index.h" // Contiene las estructuras y funciones del Índice Invertido
#include "query.h"          // Contiene las estructuras y funciones de Query
#include "queue.h" // Contiene las estructuras y funciones de la Cola de historial

// Función para limpiar el buffer de entrada después de scanf
void clear_input_buffer() {
  int c;
  while ((c = getchar()) != '\n' && c != EOF)
    ;
}

int main() {
  char search_query_buffer[256]; // Buffer para la entrada de la consulta
  int wikipedia_choice;
  int menu_choice;
  DocumentList *all_docs_list;
  Query *query = NULL;

  // Variables para almacenar la configuración de la Wikipedia elegida
  char *selected_half_path = NULL;
  int selected_num_docs = 0;
  // Inicializa la cola para el historial de búsquedas
  Queue *query_history = create_queue();
  if (!query_history) {
    fprintf(stderr, "Error al inicialitzar l'historial de búsquedes.\n");
    return 1;
  }

  // Inicializa el índice invertido
  InvertedIndex *global_inverted_index =
      inverted_index_init(1000); // Puedes ajustar este número
  if (!global_inverted_index) {
    fprintf(stderr, "Error al inicialitzar l'índex invertit.\n");
    free_queue(query_history);
    return 1;
  }

  // Inicializa el grafo de documentos
  DocumentGraph *document_graph = graph_init();
  if (!document_graph) {
    fprintf(stderr, "Error al inicialitzar el graf de documents.\n");
    inverted_index_free(global_inverted_index);
    free_queue(query_history);
    return 1;
  }

  // Bucle principal del menú de wikipedia
  while (1) {
    printf("\n           --- BENVINGUT A GOOGLE.UPF ---\n");
    printf("\n    --- SELECCIONA LA BASE DE DADES DE WIKIPEDIA ---\n\n");
    printf("                      1. 0 - 12\n");
    printf("                      2. 0 - 270\n");
    printf("                      3. 0 - 540\n");
    printf("                      4. 0 - 5400\n\n");
    printf("        * A cada base hi ha diferents documents *\n");
    printf("\nTria una opció: ");

    if (scanf("%d", &wikipedia_choice) != 1) {
      printf("Entrada no vàlida. Introdueix un número si us plau.\n");
      clear_input_buffer(); // Limpiar el buffer de entrada
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
      continue;
    }
    break;
  }
  // --- CÀRREGA DE DOCUMENTS, CONSTRUCCIÓ D'ÍNDEX I GRAF ---
  all_docs_list = load_documents(selected_half_path, selected_num_docs,
                                 global_inverted_index, document_graph);

  // Bucle principal del menú
  while (1) {
    print_queue(query_history); // Muestra las últimas búsquedas

    printf("\n         --- MENÚ DEL MOTOR DE BÚSQUEDA ---\n\n");
    printf("1. Cerca Lineal\n");
    printf("2. Cerca d'Índex Invertit \n");
    printf("3. Top 5 Documents més Rellevants Globalment\n");
    printf("0. Sortir\n");
    printf("\nTria una opció: ");

    if (scanf("%d", &menu_choice) != 1) {
      printf("Entrada no vàlida. Si us plau, introdueix un número.\n");
      clear_input_buffer(); // Limpiar el buffer de entrada
      continue;
    }
    clear_input_buffer(); // Limpiar el buffer de entrada después de leer el
                          // entero
    DocumentList *searched_docs_for_selection =
        NULL; // Usaremos esta lista para la selección de documento

    switch (menu_choice) {
    case 1: { // Búsqueda lineal (Lógica AND)
      printf("\nCerca: ");
      if (fgets(search_query_buffer, sizeof(search_query_buffer), stdin) ==
          NULL) {
        menu_choice = 0; // Salir en caso de error o EOF
        break;
      }
      search_query_buffer[strcspn(search_query_buffer, "\n")] =
          0; // Eliminar salto de línea

      if (strlen(search_query_buffer) == 0) {
        printf("Cerca buida. Tornant al menú.\n");
        continue;
      }
      enqueue(query_history, search_query_buffer);
      query = InitQuery(search_query_buffer);
      if (!query) {
        printf("No s'ha pogut inicialitzar la query.\n");
        continue;
      }
      searched_docs_for_selection =
          linear_document_search(all_docs_list, query);
      printf("\n            --- DOCUMENTS TROBATS ---\n\n");
      if (searched_docs_for_selection &&
          searched_docs_for_selection->first_document) {
        print_documents(searched_docs_for_selection, 5);
      } else {
        printf("             Ningún document trobat.\n");
      }
      break;
    }

    case 2: { // Búsqueda con índice invertido (Lógica AND y OR separada)
      printf("\nCerca: ");
      if (fgets(search_query_buffer, sizeof(search_query_buffer), stdin) ==
          NULL) {
        menu_choice = 0; // Salir en caso de error o EOF
        break;
      }
      search_query_buffer[strcspn(search_query_buffer, "\n")] =
          0; // Eliminar salto de línea

      if (strlen(search_query_buffer) == 0) {
        printf("Cerca buida. Tornant al menú.\n");
        continue;
      }
      enqueue(query_history, search_query_buffer);
      query = InitQuery(search_query_buffer);
      if (!query) { /* handle error */
        continue;
      }
      // CRIDA A LA FUNCIÓ DE CERCA AMB ÍNDEX INVERTIT
      searched_docs_for_selection = inv_index_document_search(
          all_docs_list, query,
          global_inverted_index); // <--- AQUEST ÉS EL CANVI DE NOM
      printf("\n            --- DOCUMENTS TROBATS ---\n\n");
      if (searched_docs_for_selection &&
          searched_docs_for_selection->first_document) {
        // documentsListSortedDescending ja s'ha cridat DINS de document_search
        print_documents(searched_docs_for_selection, 5);
      } else {
        printf("             Ningún document trobat.\n");
      }
      break;
    }

    case 3: { // Top 5 por relevancia global
      printf("Mostrant els 5 documentos més rellevants globalment...\n");
      // Crear una copia de la lista completa para ordenar y no modificar la
      // original
      DocumentList *temp_all_docs_copy = malloc(sizeof(DocumentList));
      if (!temp_all_docs_copy) {
        fprintf(
            stderr,
            "Error: Fallo al asignar memoria para copia de DocumentList.\n");
        continue;
      }
      temp_all_docs_copy->first_document = NULL;
      temp_all_docs_copy->size = 0;

      Document *current_original_doc = all_docs_list->first_document;
      Document *last_copy_node = NULL;
      while (current_original_doc != NULL) {
        Document *new_copy_node = (Document *)malloc(sizeof(Document));
        if (!new_copy_node) {
          fprintf(stderr,
                  "Error: Fallo al asignar memoria para nodo de copia.\n");
          free_document_list(temp_all_docs_copy); // Liberar lo que se copió
          temp_all_docs_copy = NULL;
          break;
        }
        new_copy_node->id = current_original_doc->id;
        new_copy_node->title = strdup(current_original_doc->title);
        new_copy_node->body = strdup(current_original_doc->body);
        new_copy_node->relevance = current_original_doc->relevance;
        new_copy_node->linklist =
            NULL; // No necesitamos copiar linklist para esta vista
        new_copy_node->next_document = NULL;

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
        searched_docs_for_selection =
            documentsListSortedDescending(temp_all_docs_copy);
        printf("\n              --- Documents Trobats ---\n");
        printf("\n  --- Top 5 Documents Globalment Més Rellevants ---\n\n");
        print_documents(searched_docs_for_selection, 5);
      } else {
        printf("No s'ha pogut preparar la llista per mostrar la rellevància "
               "global.\n");
      }
      break;
    }

    case 0: // Salir
      printf("\nSortint del programa...\n");
      break;

    default:
      printf("Opció invàlida. Intenta-ho de nou.\n");
      continue; // Volver al inicio del bucle
    }

    if (menu_choice == 0) {
      break; // Salir del bucle principal
    }

    // Si es la opción 2, la lista 'searched_docs_for_selection' ya se ha
    // establecido a 'and_matches' y 'or_only_matches' ya se ha liberado. Las
    // opciones 1 y 3 ya asignan 'searched_docs_for_selection' directamente.
    if (searched_docs_for_selection == NULL ||
        searched_docs_for_selection->first_document == NULL) {
      // Este mensaje ya se imprime dentro de cada case si no hay resultados.
      // Aquí solo para el caso general de que no haya nada que seleccionar.
      // Para el caso 2, el mensaje "Ningún documento encontrado" se imprime por
      // separado. No necesitamos un mensaje genérico aquí.
    } else {
      // Opcional: Preguntar al usuario si desea ver un documento específico
      printf("Tria l'ÍNDEX d'un Document (X) per veure més detalls (0-4, o -1 "
             "per fer una nova cerca): ");
      int doc_selected_idx;
      if (scanf("%d", &doc_selected_idx) != 1) {
        printf("Entrada no vàlida. Tornant al menú.\n");
        clear_input_buffer();
      } else {
        clear_input_buffer(); // Limpiar el buffer de entrada
        if (doc_selected_idx != -1) {
          print_one_document(doc_selected_idx, searched_docs_for_selection);
        }
      }
    }

    // Liberar la memoria de la query y de la lista de documentos que fueron
    // preparados para la selección
    if (query) {
      free_query(query);
      query = NULL;
    }
    if (searched_docs_for_selection) {
      free_document_list(searched_docs_for_selection);
      searched_docs_for_selection = NULL;
    }
  }

  // Liberar toda la memoria al salir del programa
  free_queue(query_history); // Libera el historial de búsquedas
  free_document_list(
      all_docs_list); // Libera la lista completa de documentos cargados
  inverted_index_free(global_inverted_index); // Libera el índice invertido
  graph_free(document_graph);                 // Libera el grafo de documentos

  return 0;
}
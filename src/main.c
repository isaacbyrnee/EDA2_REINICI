#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> // Necesario para algunas operaciones de archivo, aunque no directamente usado en este snippet

#include "documents.h" // Contiene las estructuras y funciones de Documentos
#include "inverted_index.h" // Contiene las estructuras y funciones del Índice Invertido
#include "query.h"          // Contiene las estructuras y funciones de Query
#include "queue.h" // Contiene las estructuras y funciones de la Cola de historial

int main() {
  char search[100];
  // int option;
  int menu_choice;

  Queue *query_history =
      create_queue(); // Inicializa la cola para el historial de búsquedas
  if (!query_history) {
    fprintf(stderr, "Error al inicializar el historial de búsquedas.\n");
    return 1;
  }

  // Inicializa el índice invertido.
  // El número de buckets es un parámetro importante para el rendimiento de la
  // tabla hash. Un valor primo grande o un tamaño que sea una potencia de 2 son
  // comunes. Para una práctica, 1000 o 1024 es un buen punto de partida.
  InvertedIndex *global_inverted_index =
      inverted_index_init(1000); // Puedes ajustar este número
  if (!global_inverted_index) {
    fprintf(stderr, "Error al inicializar el índice invertido.\n");
    free_queue(query_history); // Libera la cola si hubo un error aquí
    return 1;
  }

  // Carga los documentos y, al mismo tiempo, rellena el índice invertido
  // Asegúrate de que la ruta "datasets/wikipedia12/" exista y contenga los
  // archivos .txt
  DocumentList *docs_list =
      load_documents("datasets/wikipedia12/", 12, global_inverted_index);
  if (!docs_list) {
    fprintf(stderr, "Error al cargar los documentos o indexarlos.\n");
    inverted_index_free(global_inverted_index); // Libera el índice
    free_queue(query_history);                  // Libera la cola
    return 1;
  }

  // Bucle principal de búsqueda
  while (1) {
    print_queue(query_history); // Muestra las últimas búsquedas

    printf("\n--- MENU DE BUSQUEDA ---\n");
    printf("1. Buscar por Palabras (Lenta/Lineal)\n");
    printf("2. Buscar con Indice Invertido (Rapida/Hashmap + Grafo)\n");
    printf("3. Ver Top 5 Documentos por Relevancia Global (sin Query)\n");
    printf("0. Salir\n");
    printf("Elige una opcion: ");

    if (scanf("%d", &menu_choice) != 1) {
      printf("Entrada no valida. Por favor, introduce un numero.\n");
      while (getchar() != '\n')
        ; // Limpiar el buffer de entrada
      continue;
    }
    printf("Search: ");
    if (fgets(search, sizeof(search), stdin) == NULL) {
      // Error de lectura o EOF (Ctrl+D)
      break;
    }

    // Elimina el salto de línea al final de la entrada del usuario
    search[strcspn(search, "\n")] = 0;

    // Si el usuario solo presiona Enter, salimos del programa
    if (strlen(search) == 0) {
      break;
    }

    enqueue(query_history, search); // Guarda la búsqueda en el historial

    // Inicializa la estructura Query a partir de la cadena de búsqueda
    // Es crucial que InitQuery normalice las palabras de la consulta
    // de la misma manera que se normalizaron al construir el índice.
    Query *query = InitQuery(search);
    if (!query) {
      printf("No se ha podido inicializar la query.\n");
      continue; // Continuar con la siguiente iteración del bucle
    }

    // Realiza la búsqueda de documentos utilizando el índice invertido
    // docs_list se pasa para que document_search pueda obtener la información
    // completa de los documentos (título, cuerpo, etc.) a partir de sus IDs
    // obtenidos del índice.
    DocumentList *searched_docs =
        document_search(docs_list, query, global_inverted_index);

    if (!searched_docs || searched_docs->first_document == NULL) {
      printf("Ningún documento encontrado con esta búsqueda.\n");
      free_query(query); // Libera la memoria de la query
      // No hay searched_docs que liberar aquí, ya que es NULL o vacío
      continue;
    }

    printf("\n--- Documentos encontrados ---\n");
    print_all_documents(searched_docs); // Imprime los resultados de la búsqueda

    // Opcional: Preguntar al usuario si desea ver un documento específico
    printf("Seleccione el ID de un documento para ver más detalles (o -1 para "
           "nueva búsqueda): ");
    int doc_selected;
    if (scanf("%d", &doc_selected) != 1) {
      // Error de lectura o entrada no numérica, salir del bucle principal
      break;
    }
    getchar(); // Consume el carácter de nueva línea pendiente en el buffer de
               // entrada

    if (doc_selected != -1) {
      // Busca el documento seleccionado dentro de la lista de resultados de la
      // búsqueda (¡Importante! Usar 'searched_docs' para buscar por índice en
      // este caso) NOTA: print_one_document busca por índice en la lista
      // enlazada, no por ID. Si el usuario ingresa un ID, necesitarías
      // encontrar el índice de ese ID en searched_docs. Para simplificar ahora,
      // si 'doc_selected' es un ID, necesitarás una función como
      // get_document_by_id_from_list(searched_docs, doc_selected); Si
      // print_one_document espera el índice de la lista, deberías adaptar cómo
      // se obtiene el documento. Por el momento, la implementación de
      // print_one_document asume que el 'idx' es la posición en la lista. Si
      // 'doc_selected' es un ID de documento real (no el índice en la lista de
      // resultados), la llamada debería ser:
      // print_one_document_by_id(doc_selected, searched_docs); Y necesitarías
      // implementar una función que recorra 'searched_docs' hasta encontrar el
      // ID.

      // Para la forma actual de print_one_document(int idx, DocumentList *list)
      // si el usuario introduce un ID y no la posición, esta parte NO
      // funcionará como se espera. Para el Lab 1, asumo que doc_selected es la
      // posición (0, 1, 2...)
      print_one_document(doc_selected,
                         searched_docs); // Se usa 'searched_docs' aquí
    }

    // Liberar la memoria de la query y de la lista de documentos encontrados en
    // esta búsqueda
    free_query(query);
    free_document_list(searched_docs);
  }

  // Liberar toda la memoria al salir del programa
  free_queue(query_history); // Libera el historial de búsquedas
  free_document_list(
      docs_list); // Libera la lista completa de documentos cargados
  inverted_index_free(global_inverted_index); // Libera el índice invertido

  return 0;
}

// --- Funciones para liberar memoria (IMPLEMENTAR EN documents.c y query.c) ---
// Estas funciones son CRUCIALES para evitar memory leaks.

// En documents.c
void free_document_list_nodes(Document *doc_node) {
  if (doc_node == NULL)
    return;
  free_document_list_nodes(
      doc_node->next_document); // Liberar recursivamente los siguientes
  free(doc_node->title);        // Libera memoria del título
  free(doc_node->body);         // Libera memoria del cuerpo

  // Liberar la linklist asociada
  if (doc_node->linklist != NULL) {
    Link *current_link = doc_node->linklist->first;
    while (current_link != NULL) {
      Link *next_link = current_link->link_next;
      free(current_link);
      current_link = next_link;
    }
    free(doc_node->linklist);
  }
  free(doc_node); // Libera el nodo del documento
}

void free_document_list(DocumentList *list) {
  if (list == NULL)
    return;
  free_document_list_nodes(list->first_document);
  free(list); // Libera la estructura de la lista
}

/*
// En documents.c (si usas esta función en main para 'select document')
// Esta es una nueva función para obtener el documento por su ID real, no por su
posición en la lista. Document *get_document_by_id_from_list(DocumentList *list,
int id) { if (list == NULL || list->first_document == NULL) return NULL;
    Document *current = list->first_document;
    while (current != NULL) {
        if (current->id == id) {
            return current;
        }
        current = current->next_document;
    }
    return NULL; // Documento no encontrado
}

// Si la necesitas, puedes adaptar print_one_document para usarla:
void print_one_document_by_id(int doc_id, DocumentList *list) {
    Document *doc = get_document_by_id_from_list(list, doc_id);
    if (doc == NULL) {
        printf("Documento con ID %d no encontrado.\n", doc_id);
        return;
    }
    // ... (El código de impresión que ya tienes en print_one_document) ...
    printf("\nID\n%d\n\n", doc->id);
    printf("TITLE\n%s\n\n", doc->title);
    printf("RELEVANCE SCORE:\n%0.f\n\n", doc->relevance); // Asumiendo que
relevance se ha copiado o calculado printf("BODY\n%s\n\n", doc->body);
    printf("-----------------------------\n");
}
*/
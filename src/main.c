#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "documents.h"
#include "query.h"
#include "queue.h" // afegim la cua

int main() {
  char search[100];
  Queue *query_history = create_queue();

  DocumentList *docs_list = load_documents("datasets/wikipedia12/", 12);

  while (1) {
    print_queue(query_history); // mostra les últimes cerques

    printf("Search: ");
    if (fgets(search, sizeof(search), stdin) == NULL) {
      break;
    }

    // treu el salt de línia
    search[strcspn(search, "\n")] = 0;

    // si l’usuari només prem Enter, sortim
    if (strlen(search) == 0) {
      break;
    }

    enqueue(query_history, search); // guarda la cerca

    Query *query = InitQuery(search);
    if (!query) {
      printf("No s'ha pogut inicialitzar la query\n");
      continue;
    }

    DocumentList *searched_docs = document_search(docs_list, query);
    if (!searched_docs || searched_docs->first_document == NULL) {
      printf("Cap document trobat amb aquesta cerca.\n");
      continue;
    }

    print_all_documents(searched_docs);

    printf("Select document: ");
    int doc_selected;
    if (scanf("%d", &doc_selected) != 1) {
      break;
    }
    getchar(); // consume '\n'

    print_one_document(doc_selected, searched_docs); // usa searched_docs!
  }

  free_queue(query_history);
  free(docs_list);

  return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "documents.h"
#include "sample_lib.h"

int main() {
  // query:
  printf("Search: ");
  char search[100]; // per guardar el search
  scanf("%s", search);
  Query *query = InitQuery();

  // documents:
  DocumentList *docs_list = load_documents("datasets/wikipedia12/", 12);
  DocumentList *searched_docs = document_search(docs_list, query);
  print_all_documents(searched_docs);

  int doc_selected;
  printf("Select document: ");
  scanf("%d", &doc_selected);
  print_one_document(doc_selected, docs_list);

  // free allocated memory
  free(docs_list);
  return 0;
}
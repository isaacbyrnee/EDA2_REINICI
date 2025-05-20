#include "documents.h"
#include "sample_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main() {

  // printf("Search: ");
  // char search[100]; // create a variable to store the user's search
  // scanf("%s", search);

  DocumentList *docs_list = load_documents("datasets/wikipedia12/", 12);
  print_all_documents(docs_list);

  int doc_selected;
  printf("Select document: ");
  scanf("%d", &doc_selected);
  print_one_document(doc_selected, docs_list);

  // free allocated memory
  free(docs_list);
  return 0;
}
/**
 *
 * Main file 
 * 
 * This file calls the linked-list.c function files.
 *
 * Lluis Garrido, September 2018.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "linked-list.h"

/**
 *
 *  Main function. Contains a simple example using both
 *  linked-list and red-black-tree.
 *
 */

int main(int argc, char **argv)
{
  int a, maxnum, ct;

  list *l;
  list_item *l_item;
  list_data *l_data;

  if (argc != 2)
  {
    printf("Usage: %s maxnum\n", argv[0]);
    exit(1);
  }

  maxnum = atoi(argv[1]);

  printf("Test with linked-list\n");

  /* Random seed */
  srand(0);

  /* Initialize the list */
  l = (list *) malloc(sizeof(list));
  init_list(l);

  for(ct = maxnum; ct; ct--) {
    /* Generate random key to be inserted in the tree */
    a = rand() % 100 + 1;

    /* Search if the key is in the tree */
    l_data = find_list(l, a); 

    if (l_data != NULL) {

      /* We increment the number of times current item has appeared */
      l_data->num_times++;
    } else {

      /* If the key is not in the list, allocate memory for the data and
       * insert it in the list */

      l_data = malloc(sizeof(list_data));
      l_data->key = a;
      l_data->num_times = 1;

      insert_list(l, l_data);
    }
  }

  printf("Printing contents of list:\n");

  /* Dump contents to stdout */
  l_item = l->first;
  while (l_item != NULL) {
      l_data = l_item->data;
      printf("Key %03d appears %d times\n", l_data->key, l_data->num_times);
      l_item = l_item->next;
  }

  /* Delete the list */
  delete_list(l);
  free(l);

  printf("Done.\n");

  return 0;
}


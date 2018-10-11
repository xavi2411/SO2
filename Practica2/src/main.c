#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "red-black-tree.h"
#include "linked-list.h"


int main(int argc, char **argv) {
	if (argc != 4) {
	    printf("Invalid number of arguments. Usage: %s flights_file_data airports_list_file origin_airport \n", argv[0]);
	    exit(1);			
	}
	FILE *fp;
	char *flights, *airports, *origin_airport;
	char str[10];
	int lines;
	char **vector;
	int i;

	flights = argv[1];
	airports = argv[2];
	origin_airport = argv[3];

	fp = fopen(airports , "r"); 
	if(fp == NULL)
	{
		perror("Error opening file");
		return(-1);
	}
	if( fgets (str, 10, fp)!=NULL )
	{
		lines = atoi(str); 
		vector = malloc(lines * sizeof(char*));
		for (i=0; i<lines; i++){
			fgets(str, 10, fp);
			str[strlen(str)-1] = '\0';
			vector[i] = malloc(sizeof(char)*(strlen(str)+1));
			strcpy(vector[i], str);
		}
		printf("S'han llegit %d línies\n",lines);
	}
	fclose(fp);

	rb_tree *tree;
	node_data *n_data;

	/* Allocate memory for tree */
	tree = (rb_tree *) malloc(sizeof(rb_tree));

	/* Initialize the tree */
	init_tree(tree);

	for (i = 0; i < lines; i++) {
		/* Each airort is different so we don't check if node already exists in tree */

	    /* Allocate memory for the data and insert in the tree */

		n_data = malloc(sizeof(node_data));

	    /* This is the key by which the node is indexed in the tree */
		n_data->key = vector[i];

	    /* This is additional information that is stored in the tree */
		n_data->l = (list *) malloc(sizeof(list));
		init_list(n_data->l);

	    /* We insert the node in the tree */
		insert_node(tree, n_data);
	}

	/* We now dump the information of the tree to screen */

	for(i = 0; i < lines; i++)
	{
		n_data = find_node(tree, vector[i]);

		if (n_data) { 
			printf("%d)Aeroport %s\n", i,n_data->key);
		}
	}
	
	/* Delete the tree */
	delete_tree(tree);
	free(tree);

	for(i=0; i<lines; i++) {
		free(vector[i]);
	}
	free(vector);
}


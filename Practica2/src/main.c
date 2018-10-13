#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "red-black-tree.h"
#include "linked-list.h"

void tree_path_r(node *node, int *max, char *airport_max){
	if(node != NIL) {
		if (*max < node->data->l->num_items) {
			*max = node->data->l->num_items;
			strcpy(airport_max,node->data->key);
		}
		tree_path_r(node->left, max, airport_max);
		tree_path_r( node->right, max, airport_max);
	}
}

char** getInfoSeparatedByCommas(char *dades) { // obtenim els valors delay, origin, destination
	char **values = malloc(3*sizeof(char *));
	int i;
	int j = 0;
	int commaCounter = 0;
	int dada = 0;
	char delay[5] = "";
	char origin[4] = "";
	char dest[4] = "";

	for(i = 0; i < strlen(dades); i++) {
		if (dada) {
			if (dades[i] != ',') {
				if (commaCounter == 14) {
					delay[j] = dades[i];
					j = j + 1;
				}else if (commaCounter == 16) {
					origin[j] = dades[i];
					j = j + 1;
				}else if (commaCounter == 17) {
					dest[j] = dades[i];
					j = j + 1;
				}
			}else {
				j = 0;
			}
		}
		if (dades[i] == ',') {
			commaCounter += 1;
			dada = 0;
			if (commaCounter == 14 || commaCounter == 16 || commaCounter == 17) {
				dada = 1;
			}
		}
	}
	
	delay[strlen(delay)-1] = '\0';
	values[0] = malloc(strlen(delay)+1);
	strcpy(values[0], delay);
	
	origin[3] = '\0';
	values[1] = malloc(strlen(origin)+1);
	strcpy(values[1], origin);
	
	dest[3] = '\0';
	values[2] = malloc(strlen(dest)+1);
	strcpy(values[2], dest);

	return values;
}

int main(int argc, char **argv) {
	if (argc != 4) {
	    printf("Invalid number of arguments. Usage: %s flights_file_data airports_list_file origin_airport \n", argv[0]);
	    exit(1);			
	}
	FILE *fp;
	char *flights, *airports, *origin_airport;
	char str[10];
	char header[400];
	char dades[120];
	char *aeroport_origen, *aeroport_desti;
	int retard;
	int lines;
	list_data *l_data;
	list_item *l_item;
	char **vector;
	int i;

	char **info;

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
	    n_data->key = malloc(strlen(vector[i])+1);
		strcpy(n_data->key,vector[i]);

	    /* This is additional information that is stored in the tree */
		n_data->l = (list *) malloc(sizeof(list));
		init_list(n_data->l);

	    /* We insert the node in the tree */
		insert_node(tree, n_data);
	}

	/* We start to add information to the nodes of the tree */

	fp = fopen(flights , "r"); 
	if(fp == NULL)
	{
		perror("Error opening file");
		return(-1);
	}

	if( fgets (header, 400, fp)!=NULL )	/* Remove the header */
	{
		while ( fgets (dades, 120, fp)!=NULL )	/* We take whole lines until we finish the document */
		{
			dades[strlen(dades)-1] = '\0';

			info = getInfoSeparatedByCommas(dades);

			retard = atoi(info[0]);

			aeroport_origen = malloc(strlen(info[1])+1);
			strcpy(aeroport_origen, info[1]);

			aeroport_desti = malloc(strlen(info[2])+1);
			strcpy(aeroport_desti, info[2]);

			/* We search the node */
			n_data = find_node(tree, aeroport_origen); 

			if ( n_data != NULL )
			{
				l_data = find_list(n_data->l, aeroport_desti);

				if ( l_data !=NULL )	/* Case: Airport in the linked-list already */
				{	
					l_data->num_flights = l_data->num_flights + 1;
					l_data->delay = l_data->delay + retard;
				}
				else	/* Case: Airport not in the linked-list yet */
				{
					l_data = (list_data *) malloc(sizeof(list_data));
					l_data->key = malloc(strlen(aeroport_desti)+1);
					strcpy(l_data->key,aeroport_desti);
					l_data->num_flights = 1;
					l_data->delay = retard;
					insert_list(n_data->l, l_data);
				}
			}
		}
	}

	//Computació:

	/* 1) Busquem el retard mig de cada vol per l'aeroport especificat */
	n_data = find_node(tree, origin_airport);
	if ( n_data != NULL )
	{
		printf("%s\n",n_data->key);
		l_item = n_data->l->first;
		while(l_item != NULL) {
			printf(" Amb desti %s : \n",l_item->data->key);
			printf("  Retard: %f\n", l_item->data->delay / (float) l_item->data->num_flights);	
			l_item = l_item->next;
		}
	}else
	{
		printf("Unknown airport not in list.");
	}
	/* Com que tan sols quan afegim un list_item amb insert list es suma 1 a num_items, aixo hauria de funcionar */

	/* 2) Busquem l'aeroport amb més destinacions */
	int max = 0;
	int *p_max;
	p_max = &max;

	char *airport_max = malloc(3);
	tree_path_r(tree->root, p_max, airport_max);
	printf("Max: %s amb %d vols",airport_max, *p_max);
	exit(0);

	
	/* Delete the tree */
	delete_tree(tree);
	free(tree);

	/*for(i=0; i<lines; i++) {
		free(vector[i]);
	}*/
	free(vector);
}


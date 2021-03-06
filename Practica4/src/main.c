#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "red-black-tree.h"
#include "linked-list.h"


#define MAXLINE      200
#define MAGIC_NUMBER 0x0133C8F9
#define NUM_FILS     4
#define NUM_LINIES	 1000 //depen del nombre de fils si el que volem es llegir totes les linies


pthread_mutex_t clau_fitxer = PTHREAD_MUTEX_INITIALIZER;
int lines;

pthread_t ntid[NUM_FILS];

/* Definim uns estructura per passar els arguments al fil */
typedef struct args_fil {
	FILE *fitxer;
	rb_tree *tree;
} args_fil;

/**
 * 
 *  Menu
 * 
 */

int menu() 
{
    char str[5];
    int opcio;

    printf("\n\nMenu\n\n");
    printf(" 1 - Creacio de l'arbre\n");
    printf(" 2 - Emmagatzemar arbre a disc\n");
    printf(" 3 - Llegir arbre de disc\n");
    printf(" 4 - Consultar informacio de l'arbre\n");
    printf(" 5 - Sortir\n\n");
    printf("   Escull opcio: ");

    fgets(str, 5, stdin);
    opcio = atoi(str); 

    return opcio;
}


/*
* 
* Recorre l'arbre en preorder a partir d'un node donat buscant l'aeroport amb més
* destinacions, guardant el valor màxim a la direcció de memòria on apunta max i 
* guardant l'aeroport en concret a la direcció de memòria que apunta airport_max
* 
*/

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

/*
*
* A partir d'una cadena de caràcters donades, dades, es recorre la cadena per tal 
* d'obtenir els valors delay, origin i destination, les quals sabem que es troben
* a la columna 15, 17 i 18 respectivament, per tant, tenim que es troben despres
* de la 14a, 16a i 17a coma ',' de la cadena dades
* 
*/

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
					if (dades[i] == 'N' || dades[i] == '/' || dades[i] == 'A') {
						delay[j] = '0';
					}else {
						delay[j] = dades[i];
					}
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

	delay[strlen(delay)] = '\0';
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

int lectura_dades(rb_tree *tree, FILE *fp) {

	int final_fitxer = 0;
	int num_linies = -1; 

	char dades[120];
	char **info;
	char **linia_dades;
	int retard, i, j;
	char *aeroport_origen, *aeroport_desti;
	node_data *n_data;
	list_data *l_data;
	
	linia_dades = malloc(NUM_LINIES * sizeof(char*));


	pthread_mutex_lock(&clau_fitxer); //Bloquegem la clau del fitxer de dades per llegir-ne les dades

	for(i = 0; i < NUM_LINIES; i++) {
		if (fgets (dades, 120, fp) != NULL) {
			dades[strlen(dades)-1] = '\0';
			linia_dades[i] = malloc(sizeof(char)*(strlen(dades)+1));
			strcpy(linia_dades[i], dades);
		}else {
			num_linies = i; // En principi tots els fils llegeixen "NUM_LINIES" linies, però si s'arriba al final del fitxer s'hauran llegit "i" linies
			final_fitxer = 1;
			break;
		}
	}
	if (num_linies == -1) { // Si no s'ha arribat al final del fitxer, el nombre de linies llegides serà "NUM_LINIES"
		num_linies = NUM_LINIES;
	}

	pthread_mutex_unlock(&clau_fitxer); //Desbloquegem la clau del fitxer de dades un cop llegides les dades

	for(j = 0; j < num_linies; j++) {

		info = getInfoSeparatedByCommas(linia_dades[j]);

		retard = atoi(info[0]);

		aeroport_origen = malloc(strlen(info[1])+1);
		strcpy(aeroport_origen, info[1]);

		aeroport_desti = malloc(strlen(info[2])+1);
		strcpy(aeroport_desti, info[2]);

		/* We search the node */
		n_data = find_node(tree, aeroport_origen); 


		if ( n_data != NULL )
		{
			pthread_mutex_lock(&n_data->clau); //Bloquegem la clau del node per escriure'n les dades llegides

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

			pthread_mutex_unlock(&n_data->clau); //Desbloquegem la clau del node un cop llegides les dades
		}


		for(i=0; i < 3; i++) {
			free(info[i]);
		}
		free(info);
		free(aeroport_origen);
		free(aeroport_desti);
	}

	for(i = 0; i < num_linies; i++){
		free(linia_dades[i]);
	}
	free(linia_dades);
	//printf("Lectura de %d línies pel thread %d\n", num_linies, pthread_self());
	return final_fitxer;
}

void *llegir_dades_fil(void *arg) { 

	args_fil *args;
	args = (args_fil *) arg;

	rb_tree *tree = (rb_tree *) args->tree;
	FILE *fp = args->fitxer;

	int final_fitxer = 0; // Indica si fp ha arribat al final del fitxer

	while(final_fitxer == 0) {
		final_fitxer = lectura_dades(tree, fp);
	}
	
	return ((void *) 0);
}


rb_tree* creacioArbre(char *airports, char *flights) {
	FILE *fp;
	char str[10];
	int i;
	char **vector;

	char header[400];

	int err;

	fp = fopen(airports , "r"); 
	if(fp == NULL)
	{
		perror("Error opening file");
		return(NULL);
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

		/* Initialize mutex key */
		pthread_mutex_init(&n_data->clau, NULL);

	    /* We insert the node in the tree */
		insert_node(tree, n_data);
	}

	/* We start to add information to the nodes of the tree */

	fp = fopen(flights , "r"); 
	if(fp == NULL)
	{
		perror("Error opening file");
		return(NULL);
	}

	if( fgets (header, 400, fp)!=NULL )	/* Remove the header */
	{	
		args_fil* args = malloc(sizeof(args_fil));
		args->fitxer = fp;
		args->tree = tree;

		//creació dels fils
		for(i = 0; i < NUM_FILS; i++) {
			err = pthread_create(&ntid[i], NULL, llegir_dades_fil, (void *)args);
			if (err != 0) {
				printf("no puc crear el fil.\n");
				exit(1);
			}
		}
		//Esperem a qe acabin els fils
		for(i = 0; i < NUM_FILS; i++) {
			err = pthread_join(ntid[i], NULL);
			if (err != 0) {
				printf("error pthread_join amb fil %d\n", i);
				exit(1);
			}
		}
		free(args);
	}
	fclose(fp);

	for(i=0; i<lines; i++) {
		free(vector[i]);
	}
	free(vector);
	return tree;
}

void retardMig(char *origin_airport, rb_tree *tree) {
	node_data *n_data;
	list_item *l_item;

	n_data = find_node(tree, origin_airport);
	printf("Media de retardos para %s\n", origin_airport);
	if ( n_data != NULL )
	{
		printf("Retardos para el aeropuerto %s\n",n_data->key);
		l_item = n_data->l->first;
		while(l_item != NULL) {
			printf("	%s --  %.3f minutos\n", l_item->data->key, (l_item->data->delay / (float) l_item->data->num_flights));
			l_item = l_item->next;
		}
	}else
	{
		printf("El aeropuerto %s no existe en el arbol\n", origin_airport);
	}
}

void mesDestinacions(rb_tree *tree) {
	int max = 0;
	int *p_max;
	p_max = &max;

	char *airport_max = malloc(4);
	tree_path_r(tree->root, p_max, airport_max);
	
	printf("\nAeropuerto con más destinos\n");
	printf("Aeropuerto con mas destinos: %s, destinos %d\n",airport_max, *p_max);
	free(airport_max);
}

void alliberaMemoriaArbre(rb_tree *tree) {
	printf("Alliberant arbre\n");
	delete_tree(tree);
	free(tree);
}

rb_tree* carregarArbreDeDisc(rb_tree *tree, char *file) {
	FILE *fp;

	int magic;
	int i, j;
	char origen[4];
	char desti[4];
	int num_items;
	int num_vols;
	int retard;
	node_data *n_data;
	list_data *l_data;

	fp = fopen(file, "r");
	if (fp != NULL) {
		//llegim magic number
		fread(&magic, 4, 1,fp);
		if (magic == MAGIC_NUMBER) {
			//inicialitzem l'arbre
			tree = (rb_tree *) malloc(sizeof(rb_tree));
			init_tree(tree);

			//llegim el nombre de nodes de l'arbre
			fread(&lines, 4, 1, fp);
			for(i = 0; i < lines; i++) {

				n_data = malloc(sizeof(node_data));

				//llegim codi IATA origen
				fread(origen, 1, 3, fp);
				origen[3] = '\0';

				n_data->key = malloc(4);
				strcpy(n_data->key, origen);
				n_data->l = (list *) malloc(sizeof(list));
				init_list(n_data->l);

				/* Initialize mutex key */
				pthread_mutex_init(&n_data->clau, NULL);

				//llegim el numero de destins
				fread(&num_items, 4, 1, fp);

				for(j = 0; j < num_items; j++) {

					l_data = (list_data *) malloc(sizeof(list_data));

					//llegim codi IATA destí
					fread(desti, 1, 3, fp);
					desti[3] = '\0';

					l_data->key = malloc(4);
					strcpy(l_data->key, desti);

					//llegim el nombre de vols
					fread(&num_vols, 4, 1, fp);

					l_data->num_flights = num_vols;

					//llegim el retard total
					fread(&retard, 4, 1, fp);

					l_data->delay = retard;

					insert_list(n_data->l, l_data);
				}

				insert_node(tree, n_data);
			}
		}
		fclose(fp);
	}else {
		perror("Error opening file");
	}
	return tree;
}

void escriureArbre(FILE *fp, node *current) {
	if(current != NIL) {
		//escrivim codi IATA origen
		fwrite(current->data->key, 1, 3, fp);
		//escrivim el numero de destins
		fwrite(&current->data->l->num_items, 4, 1, fp);
		list_item *item = current->data->l->first;
		while(item != NULL) {
			//escrivim codi IATA destí
			fwrite(item->data->key, 1, 3, fp);
			//escrivim el nombre de vols
			fwrite(&item->data->num_flights, 4, 1, fp);
			//escrivim el retard total
			fwrite(&item->data->delay, 4, 1, fp);
			item = item->next;
		}
		escriureArbre(fp, current->left);
		escriureArbre(fp, current->right);
	}	
}

void guardarArbreDisc(rb_tree *tree, char *file) {
	FILE *fp;
	int magic = MAGIC_NUMBER;

	node *current;


	fp = fopen(file, "w");

	if (fp != NULL) {
		//escrivim magic number
		fwrite(&magic, 4, 1, fp);
		//escrivim el nombre de nodes de l'arbre
		fwrite(&lines, 4, 1, fp);

		current = tree->root;

		escriureArbre(fp, current);
		fclose(fp);

	}else {
		perror("Error opening file");
	}
}



int main(int argc, char **argv)
{
    char str1[MAXLINE], str2[MAXLINE];
    int opcio;

    struct timeval tv1, tv2; // Cronologic
	clock_t t1, t2; // CPU

    rb_tree *tree = NULL;

    if (argc != 1)
        printf("Opcions de la linia de comandes ignorades\n");

    do {
        opcio = menu();
        printf("\n\n");

        switch (opcio) {
            case 1:
                printf("Introdueix fitxer que conte llistat d'aeroports: ");
                fgets(str1, MAXLINE, stdin);
                str1[strlen(str1)-1]=0;

                printf("Introdueix fitxer de dades: ");
                fgets(str2, MAXLINE, stdin);
                str2[strlen(str2)-1]=0;

                if (tree != NULL) {
                	alliberaMemoriaArbre(tree);
                	tree = NULL;
                }

                gettimeofday(&tv1, NULL);
				t1 = clock();

                tree = creacioArbre(str1,str2);

                gettimeofday(&tv2, NULL);
				t2 = clock();

				printf("Temps de CPU: %f seconds\n", (double)(t2 - t1) / (double) CLOCKS_PER_SEC);
				printf("Temps cronologic = %f seconds\n", (double) (tv2.tv_usec - tv1.tv_usec) / 1000000 + (double) (tv2.tv_sec - tv1.tv_sec));

                break;

            case 2:
                printf("Introdueix el nom de fitxer en el qual es desara l'arbre: ");
                fgets(str1, MAXLINE, stdin);
                str1[strlen(str1)-1]=0;

                if (tree != NULL) {
                	guardarArbreDisc(tree, str1);
                }else {
                	printf("No hi ha arbre creat");
                }

                break;

            case 3:
                printf("Introdueix nom del fitxer que conte l'arbre: ");
                fgets(str1, MAXLINE, stdin);
                str1[strlen(str1)-1]=0;

                if (tree != NULL) {
                	alliberaMemoriaArbre(tree);
                	tree = NULL;
                }

                tree = carregarArbreDeDisc(tree, str1);

                /* Falta codi */

                break;

            case 4:
                printf("Introdueix aeroport per cercar retard o polsa enter per saber l'aeroport amb mes destins: ");
                fgets(str1, MAXLINE, stdin);
                str1[strlen(str1)-1]=0;
                if(tree != NULL) {
	                if (strlen(str1) > 0) {
	                	retardMig(str1, tree);
	                }else {
	                	mesDestinacions(tree);
	                }
	            }else {
	                printf("No hi ha arbre creat\n");
	            }
                break;

            case 5:
            	if (tree != NULL) {
            		alliberaMemoriaArbre(tree);
                }else {
	                printf("No hi ha arbre creat\n");
                }
                /* Falta codi */

                break;

            default:
                printf("Opcio no valida\n");

        } /* switch */
    }
    while (opcio != 5);

    return 0;
}
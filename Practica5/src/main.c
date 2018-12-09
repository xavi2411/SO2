#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "red-black-tree.h"
#include "linked-list.h"


#define MAXLINE      200
#define MAGIC_NUMBER 0x0133C8F9
#define NUM_FILS     4		// F: Nombre de consumidors
#define NUM_LINIES	 1000	// N: Nombre de línies per bloc a llegir pel productor
#define NUM_CELLS	 8		// B: Nombre de blocs del buffer


pthread_mutex_t clau_buffer = PTHREAD_MUTEX_INITIALIZER;	// Clau mutex del buffer
pthread_cond_t cua_prod, cua_cons;							// Variable condicional del productor i dels consumidors


/* Definim una estructura per passar els arguments al fil */

typedef struct cell {
    char **linies;
    int mida;
} cell;

typedef struct buffer {
    struct cell *cell[NUM_CELLS];
    int num_elements;
    int final_fitxer;
} buffer;

typedef struct args_prod {
	FILE *fitxer;
	buffer *buff;
} args_prod;


typedef struct args_cons {
	rb_tree *tree;
	buffer *buff;
	int cons_id;
} args_cons;





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

/**
 *
 *
 *
 *
 *
 *	Practica 5
 *
 *
 *
 *
 *
 *
 *
 */

void *lectura_fitxer(void *arg) {

	args_prod *args = (args_prod *) arg;
	FILE *fp = args->fitxer;
	buffer *buff = args->buff;

	free(args);
	
	char dades[120];
	int i;
	int final_fitxer = 0;

	fgets (dades, 120, fp);

	cell *tmp;
	cell *cela = malloc(sizeof(struct cell));

	while(!buff->final_fitxer) { // El Productor acaba quan ha llegit tot el fitxer
		cela->linies = malloc(NUM_LINIES * sizeof(char*));
		cela->mida = NUM_LINIES;

		for(i = 0; i < NUM_LINIES; i++) {
			if (fgets (dades, 120, fp) != NULL) {
				dades[strlen(dades)-1] = '\0';
				cela->linies[i] = malloc(sizeof(char)*(strlen(dades)+1));
				strcpy(cela->linies[i], dades);
			}else { // El productor arriba al final del fitxer sense arribar a llegit N línies
				final_fitxer = 1;
				cela->mida = i;
				break;
			}
		}

		// Bloquegem la clau del buffer per escriure-hi dades
		pthread_mutex_lock(&clau_buffer);
		while (buff->num_elements == NUM_CELLS) { // El buffer és ple
			// Esperem fins que un consumidor enviï un senyal indicant que el buffer ja no és ple
			pthread_cond_wait(&cua_prod, &clau_buffer);
		}
		for(i = 0; i < NUM_CELLS; i++) {
			if (buff->cell[i]->mida == -1) { //	Busco cela buida
				tmp = buff->cell[i];
				buff->cell[i] = cela;
				cela = tmp;
				break;
			}
		}

		if (buff->num_elements == 0) {
			// Si el buffer estava buit, desbloquegem un Consumidor
			pthread_cond_broadcast(&cua_cons);
		}
		buff->num_elements += 1;
		buff->final_fitxer = final_fitxer;

		pthread_mutex_unlock(&clau_buffer); //Desbloquegem la clau del buffer de dades un cop escrites
	}

	// Alliberem memòria
	free(cela);
	return ((void *) 0);
}

void *processament_dades(void *arg) {

	args_cons *args = (args_cons *) arg;
	rb_tree *tree = args->tree;
	buffer *buff = args->buff;
	int id = args->cons_id;

	free(args);
	
	char **info;
	int retard, i, j;
	char *aeroport_origen, *aeroport_desti;
	node_data *n_data;
	list_data *l_data;

	cell *tmp;
	cell *cela = malloc(sizeof(struct cell));
	cela->mida = -1;

	while(!buff->final_fitxer || buff->num_elements != 0) { // La tasca del consumidor acaba quan s'arriba a final de fitxer i el buffer és buit

		// Bloquegem la clau del buffer per llegir dades
		pthread_mutex_lock(&clau_buffer); 
		while (buff->num_elements == 0) { // El buffer esta buit
			if (buff->final_fitxer) { // Si ja s'ha llegit tot el fitxer i el buffer es buit, evitem que el fil es bloquegi i el fem acabar
				pthread_mutex_unlock(&clau_buffer); //Desbloquegem la clau del buffer de dades un cop llegides
				free(cela);
				return ((void *) 0);
			}
			// El Consumidor espera el senyal que indica que el buffer deixa d'estar buit
			pthread_cond_wait(&cua_cons, &clau_buffer);

		}
		for(i = 0; i < NUM_CELLS; i++) {
			if (buff->cell[i]->mida != -1) { // Busco cela amb dades
				tmp = cela;
				cela = buff->cell[i];
				buff->cell[i] = tmp;
				break;
			}
		}
		if (buff->num_elements == NUM_CELLS) { 
			// Si el buffer estava ple, desbloquegem el Productor
			pthread_cond_signal(&cua_prod);
		}
		buff->num_elements -= 1;
		pthread_mutex_unlock(&clau_buffer); //Desbloquegem la clau del buffer de dades un cop llegides

		/* Ara tenim les dades a processar a la variable local cela */
		for(j = 0; j < cela->mida; j++) {

			info = getInfoSeparatedByCommas(cela->linies[j]);

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

				if ( l_data != NULL )	/* Case: Airport in the linked-list already */
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

			// Alliberem memòria
			for(i=0; i < 3; i++) {
				free(info[i]);
			}
			free(info);
			free(aeroport_origen);
			free(aeroport_desti);

			free(cela->linies[j]);
		}
		free(cela->linies);
		cela->mida = -1;
	}

	// Alliberem memòria
	free(cela);

	return ((void *) 0);
}


rb_tree* creacioArbre(char *airports, char *flights) {

	FILE *fp;
	char str[10];
	int i;
	char **vector;
	char header[400];
	int err;
	int lines;

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
	tree->num_elements = lines;


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
		// Inicialitzem buffer
		buffer *buff = malloc(sizeof(buffer));
		for(i = 0; i < NUM_CELLS; i++) {
			buff->cell[i] = malloc(sizeof(cell));
			buff->cell[i]->mida = -1;
		}

		buff->num_elements = 0;
		buff->final_fitxer = 0;

		// Inicialitzem els arguments del Productor
		args_prod* args_p = malloc(sizeof(args_prod));
		args_p->fitxer = fp;
		args_p->buff = buff;		

		// Inicialitzem els arguments dels Consumidors
		args_cons* args_c;

		// Definim els fils: Productor i Consumidors
		pthread_t productor;				// Fil productor
		pthread_t consumidors[NUM_FILS];	// Fils consumidors

		// Inicialitzem variables condicionals per a cada fil
		pthread_cond_init(&cua_prod, NULL);
		pthread_cond_init(&cua_cons, NULL);


		// Creem fil Productor
		err = pthread_create(&productor, NULL, lectura_fitxer, (void *)args_p);
		if (err != 0) {
			printf("no puc crear el productor.\n");
			exit(1);
		}

		// Creem fils Consumidors
		for(i = 0; i < NUM_FILS; i++) {
			//consumidors_bloquejats[i] = 0;

			args_c = malloc(sizeof(args_cons));
			args_c->tree = tree;
			args_c->buff = buff;
			args_c->cons_id = i;
			err = pthread_create(&consumidors[i], NULL, processament_dades, (void *)args_c);
			if (err != 0) {
				printf("no puc crear el consumidor %d.\n", i);
				exit(1);
			}
		}

		// Esperem a què acabi el fil Productor
		err = pthread_join(productor, NULL);
		if (err != 0) {
			printf("error pthread_join amb productor\n");
			exit(1);
		}

		// Esperem a què acabin els fils Consumidors
		for(i = 0; i < NUM_FILS; i++) {
			err = pthread_join(consumidors[i], NULL);
			if (err != 0) {
				printf("error pthread_join amb consumidor %d\n", i);
				exit(1);
			}
		}

		// Destruim variables condicionals
		pthread_cond_destroy(&cua_prod);
		pthread_cond_destroy(&cua_cons);

		// Alliberem memòria del buffer
		for(i = 0; i < NUM_CELLS; i++) {
	       	free(buff->cell[i]);
		}
	   	free(buff);
	}
	// Tanquem el fitxer
	fclose(fp);

	// Alliberem memoria
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

	int magic, lines;
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
			tree->num_elements = lines;

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
		fwrite(&tree->num_elements, 4, 1, fp);

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
	if (argc != 4) {
	    printf("Invalid number of arguments. Usage: %s flights_file_data airports_list_file origin_airport \n", argv[0]);
	    exit(1);			
	}
	FILE *fp;
	char str[10];
	int lines;
	char **vector;
	int i;

	fp = fopen("aeroports/aeroports.csv" , "r"); 
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
			vector[i] = malloc(sizeof(char)*(strlen(str)-1));
			strcpy(vector[i], str);
		}
		printf("S'han llegit %d línies\n",lines);
	}
	fclose(fp);

	for(i=0; i<lines; i++){
		free(vector[i]);
	}
	free(vector);
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int compara(const void *p1, const void *p2)
{
	int value;
	char *str1, *str2;
	str1 = *((char **) p1);
	str2 = *((char **) p2);
	int len1 = strlen(str1);
	int len2 = strlen(str2);
	if (len1 < len2) {
		value = -1;
	}else if(len1 == len2) {
		value = 0;
	}else {
		value = 1;
	}
	return value;
}

int main()
{
	FILE *fp;
	char str[100];
	int lines;
	char **vector;
	int i;
	fp = fopen("dades/strings.txt" , "r");
	if(fp == NULL)
	{
		perror("Error opening file");
		return(-1);
	}
	if( fgets (str, 100, fp)!=NULL )
	{
		lines = atoi(str); 
		vector = malloc(lines*sizeof(char*));
		for (i=0; i<lines; i++){
			fgets(str, 100, fp);
			str[strlen(str)-1] = '\0';
			vector[i] = malloc(sizeof(char)*(strlen(str)-1));
			strcpy(vector[i], str);
		}
		printf("S'han llegit %d línies\n",lines);
	}
	fclose(fp);
	
	qsort(vector, lines, sizeof(char*), compara);

	char *p;
	int lines_to_show = 100;
	if (lines<lines_to_show){
		lines_to_show = lines;
	}
	printf("Mostrant %d elements ordenats pel seu contingut: \n", lines_to_show);
	for(i=0; i<lines_to_show; i++){
		printf("%s\n", vector[i]);
		free(vector[i]);
	}
	free(vector);
	return(0);
}

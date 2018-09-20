#include <stdio.h>
#include <stdlib.h>

int compara(const void *p1, const void *p2) {
	int *num1, *num2;
	num1 = (int *) p1;
	num2 = (int *) p2;

	if (*num1 < *num2)
		return 1;
	if (*num1 > *num2)
		return -1;
	return 0;
}

int main() {
    FILE *fp;
	char str[100];
	fp = fopen("dades/integers.txt" , "r"); 
	if(fp == NULL)
	{
      perror("Error opening file");
	return(-1); 
	}
	if( fgets (str, 100, fp)!=NULL ) {
		int linies = atoi(str); 
		int *p_int = (int*) malloc(linies*sizeof(int));
		int i, str_int;
		for (i=0; i<linies; i++){
			str_int = atoi(fgets(str, 100, fp));
			p_int[i] = str_int;
		}

		qsort(p_int, linies, sizeof(int), compara);

		printf("Ordenades %d linies. \n", linies);
		int linies_a_llegir;
		if (linies<50){
			linies_a_llegir = linies;
		}else{
			linies_a_llegir = 50;
		}
		printf("Mostrant %d elements: \n", linies_a_llegir);
		for(i=0;i<linies_a_llegir;i++){
			printf("%d \n",p_int[i]);
		}

		free(p_int);
   	}
	fclose(fp);
	return(0); 
}

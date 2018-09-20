#include <stdio.h>
#include <stdlib.h>

int compara(const void *p1, const void *p2) {
	double *num1, *num2;
	num1 = (double *) p1;
	num2 = (double *) p2;

	if (*num1 < *num2)
		return 1;
	if (*num1 > *num2)
		return -1;
	return 0;
}

int main() {
    FILE *fp;
	char str[100];
	fp = fopen("dades/float.txt" , "r"); 
	if(fp == NULL)
	{
      perror("Error opening file");
	return(-1); 
	}
	if( fgets (str, 100, fp)!=NULL ) {
		int linies = atoi(str); 
		double *p_double = (double*) malloc(linies*sizeof(double));
		int i;
		double str_double;
		for (i=0; i<linies; i++){
			str_double = atof(fgets(str, 100, fp));
			p_double[i] = str_double;
		}

		qsort(p_double, linies, sizeof(double), compara);

		printf("Ordenades %d linies. \n", linies);
		int linies_a_llegir;
		if (linies<50){
			linies_a_llegir = linies;
		}else{
			linies_a_llegir = 50;
		}
		printf("Mostrant %d elements: \n", linies_a_llegir);
		for(i=0;i<linies_a_llegir;i++){
			printf("%f \n",p_double[i]);
		}

		free(p_double);
   	}
	fclose(fp);
	return(0); 
}

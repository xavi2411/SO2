#include <stdio.h>
#include <stdlib.h>

int compara(const void *p1, const void *p2) {
	float *num1, *num2;
	num1 = (float *) p1;
	num2 = (float *) p2;

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
		float *p_float = (float*) malloc(linies*sizeof(float));
		int i;
		float str_float;
		for (i=0; i<linies; i++){
			str_float = atof(fgets(str, 100, fp));
			p_float[i] = str_float;
		}

		qsort(p_float, linies, sizeof(float), compara);

		printf("Ordenades %d linies. \n", linies);
		int linies_a_llegir;
		if (linies<50){
			linies_a_llegir = linies;
		}else{
			linies_a_llegir = 50;
		}
		printf("Mostrant %d elements: \n", linies_a_llegir);
		for(i=0;i<linies_a_llegir;i++){
			printf("%f \n",p_float[i]);
		}

		free(p_float);
   	}
	fclose(fp);
	return(0); 
}

#include "utils.h"

void readFromFile(char* fileName, int **fileIds, int **content, int dictionarySize, int *nof){
	FILE *fp = fopen(fileName, "r");
	char voidBuffer[10000];
	int numberOfFiles = 0;
	while( fgets(voidBuffer, 10000, fp) > 0) {
		numberOfFiles++;
	}

	(*fileIds) = (int *)malloc(sizeof(int) * numberOfFiles);
	(*content) = (int *)malloc(sizeof(int) * numberOfFiles * dictionarySize);

	rewind(fp);

	int i;
	int j;

	for(i=0;i<numberOfFiles;i++) {
		fscanf(fp, " %d%c", &((*fileIds)[i]), voidBuffer);
		for(j=0; j<dictionarySize; j++) {
			int temp;
			fscanf(fp, " %d", &((*content)[i*dictionarySize + j]));
		}
	}

	*nof = numberOfFiles;

	fclose(fp);
}

void readQueryFromFile(char *fileName, int **content, int dictionarySize) {
	FILE *fp = fopen(fileName, "r");

	(*content) = (int *)malloc(sizeof(int) * dictionarySize);

	int i;
	for(i = 0; i < dictionarySize; i++) {
		fscanf(fp, " %d", &((*content)[i]));
	}

	fclose(fp);
}
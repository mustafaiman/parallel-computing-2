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

void modifiedQuickSort(int references[], int values[], int first,int last) {
    int pivot, j, tempReference, tempValue,i;
    
    if(first<last){
        pivot=first;
        i=first;
        j=last;
        
        while(i<j){
            while(values[i] <= values[pivot] && i<last)
                i++;
            while(values[j] > values[pivot])
                j--;
            if(i<j){
                tempReference=references[i];
                references[i]=references[j];
                references[j]=tempReference;
                
                tempValue = values[i];
                values[i] = values[j];
                values[j] = tempValue;
            }
        }
        
        tempReference = references[pivot];
        references[pivot] = references[j];
        references[j] = tempReference;
        
        tempValue = values[pivot];
        values[pivot] = values[j];
        values[j] = tempValue;
        
        modifiedQuickSort(references, values, first, j-1);
        modifiedQuickSort(references, values, j+1, last);
    }
}

void calculateSimilarities(int *content, int *query, int *similarities, int numberOfFiles, int dictionarySize) {
    int i;
    int j;
    for(i=0;i< numberOfFiles; i++) {
        similarities[i] = 0;
        for(j=0;j<dictionarySize;j++) {
            similarities[i] += pow((double) content[i*dictionarySize + j], (double)query[j]);
        }
    }
}
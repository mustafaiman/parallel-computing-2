#include<stdio.h>
#include<stdlib.h>
#include<math.h>

void readFromFile(char* fileName, int **fileIds, int **content, int dictionarySize, int *numberOfFiles);

void readQueryFromFile(char *fileName, int **content, int dictionarySize);

void modifiedQuickSort(int references[], int values[], int first,int last);

void calculateSimilarities(int *content, int *query, int *similarities, int numberOfFiles, int dictionarySize);
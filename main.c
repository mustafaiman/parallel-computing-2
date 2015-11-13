#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>

#include "utils.h"

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

int main(int argc, char *argv[]) {
	MPI_Init(&argc, &argv);
	/*
	1 => dictinarySize
	2 => kvalue
	3 => documents.txt
	4 => query.txt
	*/
	int MY_ID, NUM_PROCS;
	MPI_Comm_size(MPI_COMM_WORLD, &NUM_PROCS);
	MPI_Comm_rank(MPI_COMM_WORLD, &MY_ID);

	int dictionarySize = atoi(argv[1]);
	int kvalue = atoi(argv[2]);
	int numberOfFiles;

	int partitionSize;
	int *partitionSimilarities;
	int *partitionFileIds;

	if(MY_ID == 0) {
		int *fileIds;
		int *content;
		int *query;
		int numberOfFiles;
		int i;
		int j;

		readFromFile(argv[3], &fileIds, &content, dictionarySize, &numberOfFiles);
		readQueryFromFile(argv[4], &query, dictionarySize);

		for(i = 0; i<numberOfFiles; i++) {
			printf("%d : ", fileIds[i]);
			for (j = 0; j < dictionarySize; j++)
			{
				printf("%d ", content[i*dictionarySize + j]);
			}
			printf("\n");
		}

		for(i = 0; i<dictionarySize; i++) {
			printf("%d ", query[i]);
		}
		printf("\n");

		int *similarities = (int *)malloc(sizeof(int) * numberOfFiles);

		calculateSimilarities(content, query, similarities, numberOfFiles, dictionarySize);

		printf("Similarities\n");
		for(i=0; i<numberOfFiles; i++) {
			printf("%d\n", similarities[i]);
		}

		partitionSize = numberOfFiles / NUM_PROCS + (numberOfFiles % NUM_PROCS == 0 ? 0 : 1);
		int offset = partitionSize;

		partitionSimilarities = (int *)malloc(sizeof(int) * partitionSize);
		memcpy(partitionSimilarities, similarities, partitionSize * sizeof(int));

		partitionFileIds = (int *)malloc(sizeof(int) * partitionSize);
		memcpy(partitionFileIds, fileIds, partitionSize * sizeof(int));

		for(i=1;i< numberOfFiles % NUM_PROCS;i++) {
			int temporaryPartitionCount = numberOfFiles / NUM_PROCS + 1;
			MPI_Send(&temporaryPartitionCount, 1, MPI_INT, i, 0xACE5, MPI_COMM_WORLD);
			MPI_Send(similarities + offset, temporaryPartitionCount, MPI_INT, i, 0xACE5, MPI_COMM_WORLD);
			MPI_Send(fileIds + offset, temporaryPartitionCount, MPI_INT, i, 0xACE5, MPI_COMM_WORLD);
			offset += temporaryPartitionCount;
		}
		for(i=numberOfFiles % NUM_PROCS; i< NUM_PROCS;i++) {
			int temporaryPartitionCount = numberOfFiles / NUM_PROCS;
			MPI_Send(&temporaryPartitionCount, 1, MPI_INT, i, 0xACE5, MPI_COMM_WORLD);
			MPI_Send(similarities + offset, temporaryPartitionCount, MPI_INT, i, 0xACE5, MPI_COMM_WORLD);
			MPI_Send(fileIds + offset, temporaryPartitionCount, MPI_INT, i, 0xACE5, MPI_COMM_WORLD);
			offset += temporaryPartitionCount;
		}

	} else {

		MPI_Recv(&partitionSize, 1, MPI_INT, 0, 0xACE5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		partitionSimilarities = (int *)malloc(sizeof(int) * partitionSize);
		MPI_Recv(partitionSimilarities, partitionSize, MPI_INT, 0, 0xACE5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		partitionFileIds = (int *)malloc(sizeof(int) * partitionSize);
		MPI_Recv(partitionFileIds, partitionSize, MPI_INT, 0, 0xACE5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	}
	int i;
	printf("Process %d : ", MY_ID);
	for(i=0; i<partitionSize; i++) {
		printf("p%d %d ",MY_ID, partitionSimilarities[i]);
	}
	for(i=0; i<partitionSize; i++) {
		printf("p%d %d ",MY_ID, partitionFileIds[i]);
	}
	printf("\n");

	MPI_Finalize();
	return 0;
}
#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>

#include "utils.h"

#define MAX(x,y) ((x) > (y) ? (x) : (y))

void kreduce(int *kleast, int *myids, int *myvals, int k, int world_size, int myid) {
    int *recvReferences;
    int *recvValues;
    if( myid == 0 ) {
        recvReferences = (int *)malloc(sizeof(int) * k * world_size);
        recvValues = (int *)malloc(sizeof(int) * k * world_size);
    }
    
    MPI_Gather(myids, k, MPI_INT, recvReferences, k, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(myvals, k, MPI_INT, recvValues, k, MPI_INT, 0, MPI_COMM_WORLD);
    
    if(myid == 0) {
        printf("Results accumulated\n");
        for (int i=0; i< world_size * k; i++) {
            printf("%d %d\n", recvReferences[i], recvValues[i]);
        }
        
        int *indexes = (int *)malloc(sizeof(int) * world_size);
        memset(indexes, 0, sizeof(int) * world_size);
        
        for(int i=0; i<k; i++) {
            int minimumValue = INT_MAX;
            int minimumProcess = 0;
            for (int j=0; j<world_size; j++) {
                if( indexes[j] < k && minimumValue >= recvValues[k*j + indexes[j]]) {
                    minimumValue = recvValues[k*j + indexes[j]];
                    minimumProcess = j;
                }
            }
            kleast[i] = recvReferences[minimumProcess*k + indexes[minimumProcess]];
            indexes[minimumProcess]++;
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
			printf("%d %d\n", fileIds[i], similarities[i]);
		}

		partitionSize = numberOfFiles / NUM_PROCS + (numberOfFiles % NUM_PROCS == 0 ? 0 : 1);
		int offset = partitionSize;

		partitionSimilarities = (int *)malloc(sizeof(int) * MAX(partitionSize, kvalue));
		memcpy(partitionSimilarities, similarities, partitionSize * sizeof(int));

		partitionFileIds = (int *)malloc(sizeof(int) * MAX(partitionSize, kvalue));
		memcpy(partitionFileIds, fileIds, partitionSize * sizeof(int));

		for(i=1;i< numberOfFiles % NUM_PROCS;i++) {
			int temporaryPartitionCount = numberOfFiles / NUM_PROCS + 1;
			MPI_Send(&temporaryPartitionCount, 1, MPI_INT, i, 0xACE5, MPI_COMM_WORLD);
			MPI_Send(similarities + offset, temporaryPartitionCount, MPI_INT, i, 0xACE5, MPI_COMM_WORLD);
			MPI_Send(fileIds + offset, temporaryPartitionCount, MPI_INT, i, 0xACE5, MPI_COMM_WORLD);
			offset += temporaryPartitionCount;
		}
        for(i=numberOfFiles % NUM_PROCS; i< NUM_PROCS;i++) {
            if( i == 0)
                continue;
			int temporaryPartitionCount = numberOfFiles / NUM_PROCS;
			MPI_Send(&temporaryPartitionCount, 1, MPI_INT, i, 0xACE5, MPI_COMM_WORLD);
			MPI_Send(similarities + offset, temporaryPartitionCount, MPI_INT, i, 0xACE5, MPI_COMM_WORLD);
			MPI_Send(fileIds + offset, temporaryPartitionCount, MPI_INT, i, 0xACE5, MPI_COMM_WORLD);
			offset += temporaryPartitionCount;
		}

	} else {

		MPI_Recv(&partitionSize, 1, MPI_INT, 0, 0xACE5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		partitionSimilarities = (int *)malloc(sizeof(int) * MAX(partitionSize, kvalue));
		MPI_Recv(partitionSimilarities, partitionSize, MPI_INT, 0, 0xACE5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		partitionFileIds = (int *)malloc(sizeof(int) * MAX(partitionSize, kvalue));
		MPI_Recv(partitionFileIds, partitionSize, MPI_INT, 0, 0xACE5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

	}
	int i;
    /*
	for(i=0; i<partitionSize; i++) {
		printf("process-similarities%d %d\n",MY_ID, partitionSimilarities[i]);
	}
	for(i=0; i<partitionSize; i++) {
		printf("process-fileIds%d %d\n",MY_ID, partitionFileIds[i]);
	}
	printf("\n");
    */
    printf("Sorting\n");
    modifiedQuickSort(partitionFileIds, partitionSimilarities, 0, partitionSize-1);
    
    //If k is larger than partitionSize, this is neccessary to prevent garbage cluttering minimum selection in kreduce
    for(int i=partitionSize; i<kvalue; i++) {
        partitionSimilarities[i] = INT_MAX;
    }
    
    for(i=0; i<partitionSize; i++) {
        printf("p%d partiiont-size=%d %d %d\n",MY_ID, partitionSize, partitionSimilarities[i], partitionFileIds[i]);
    }
    int *kleast;
    if( MY_ID == 0) {
        kleast = (int *)malloc(sizeof(int) * kvalue);
    }
    
    kreduce(kleast, partitionFileIds, partitionSimilarities, kvalue, NUM_PROCS, MY_ID);
    
    if(MY_ID == 0 ) {
        printf("Results\n");
        for (int i=0; i<kvalue; i++) {
            printf("%d\n", kleast[i]);
        }
    }
    
	MPI_Finalize();
	return 0;
}
// hello.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	int rows = 10, cols = 10, generations = 10;

	char filename[255];
	strcpy(filename, "./");

	char *ptr = NULL;
	if (argc > 1) rows = strtol(argv[1], &ptr, 10);
	if (argc > 2) cols = strtol(argv[2], &ptr, 10);
	if (argc > 3) strcpy(filename, argv[3]);
	else strcpy(filename, "life.txt");
	if (argc > 4) generations = strtol(argv[4], &ptr, 10);

	rows += 2;
	cols += 2;

	char *World = (char *)malloc(rows * cols * sizeof(char));
	char *CopyWorld = (char *)malloc(rows * cols * sizeof(char));
	memset(World, '-', rows * cols);

	FILE *fp;
	fp = fopen(filename, "r");

	char buffer[cols];
	int r,c;

	for (r = 1; r < rows-1; r++) {
		char *endOfFile = fgets(buffer, cols+1, fp);

		for (c = 1; c < cols-1; c++) {
			if (endOfFile != NULL && c < strlen(buffer)+1) {
				if (buffer[c-1] == '*') World[r*rows + c] = '*';
			}
		}
		while (endOfFile != NULL && buffer[strlen(buffer)-1] != '\n') {
			endOfFile = fgets(buffer, cols+1, fp);
		}
	}
	fclose(fp);

	int currentGen = 0;
	while (currentGen < generations+1) {
		printf("Generation %d:\n", currentGen++);

		int i,j;
		for (i = 1; i < rows-1; i++) {
			for (j = 1; j < cols-1; j++) {
				printf("%c", World[i*rows + j]);
			}
			printf("\n");
		}
		printf("================================\n");

		for (i = 0; i < rows * cols; i++) {
			CopyWorld[i] = World[i];
		}

		for (r = 1; r < rows-1; r++) {
			for (c = 1; c < cols-1; c++) {
				int neighbors = 0;
				if (CopyWorld[(r-1)*rows + c] == '*') neighbors++; // Left
				if (CopyWorld[(r+1)*rows + c] == '*') neighbors++; // Right
				if (CopyWorld[r*rows + (c+1)] == '*') neighbors++; // Up
				if (CopyWorld[r*rows + (c-1)] == '*') neighbors++; // Down
				if (CopyWorld[(r-1)*rows + (c+1)] == '*') neighbors++; // Upper left corner
				if (CopyWorld[(r+1)*rows + (c+1)] == '*') neighbors++; // Upper right corner
				if (CopyWorld[(r-1)*rows + (c-1)] == '*') neighbors++; // Lower left corner
				if (CopyWorld[(r+1)*rows + (c-1)] == '*') neighbors++; // Lower right corner

				if (CopyWorld[r*rows + c] == '*' && (neighbors < 2 || neighbors > 3)) 
					World[r*rows + c] = '-';

				if (CopyWorld[r*rows + c] == '-' && neighbors == 3) 
					World[r*rows + c] = '*';
			}
		}
	}

	free(World);
	free(CopyWorld);
	return 0;
}
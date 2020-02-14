#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

extern char **environ;

// Prototypes

void freeEnv(char ***newEnv, int err);
	// Free the passed in heap environment from memory
	// If err is 1 then free up memory then display perror and exit otherwise just do free

void modifyEnv(char ***newEnv, char *pair, char *pch);
	// pair - string pair given
	// pch - place where '=' is found in pair
	// If an exisiting key exist, replace it with the pair, otherwise expand the passed in environment

void copyEnv(char ***newEnv);
	// Copy environ into the passed in environment

void createEnv(char ***newEnv, int pairIndex, char *argv[]);
	// pairIndex - starting index to use for argv
	// Copy all the pairs onto the passed environment

void displayEnv(char ***newEnv);
	// Dislay the passed in environment

void execCommand(char ***newEnv, int argc, char *argv[], int cmdIndex);
	// cmdIndex - starting index to use for argv
	// Parse the command and arguments from argv and run exec on it


int main(int argc, char *argv[]) {
	int index = 1;
	bool modify = true;

	if (argc == 1) {
		displayEnv(&environ);
		return 0;
	}
	
	char **newEnv = calloc(1, sizeof(char*));
	if (newEnv == NULL) 
		freeEnv(&newEnv, 1);

	if (strcmp(argv[index],"-i") == 0) {
		modify = false;
		index++;
	}
	else copyEnv(&newEnv);

	while (argv[index] != NULL) {
		char *pch = strchr(argv[index], '=');
		if (pch != NULL) {
			if (modify) modifyEnv(&newEnv, argv[index], pch);
			index++;
		}
		else break;
	}

	if (!modify && index != 2) createEnv(&newEnv, index, argv);

	if (argv[index] == NULL) {
		displayEnv(&newEnv);
	}

	// char **args = calloc(argc - index + 1, sizeof(char*));
	// if (args == NULL)
	// 	freeEnv(&newEnv, 1);

	execCommand(&newEnv, argc, argv, index);

	// int offset = index;
	// while (argv[index] != NULL) {
	// 	args[index - offset] = argv[index];
	// 	index++;
	// }

	// pid_t child_pid = fork();
	// if (child_pid < 0) {
	// 	perror("Error from calling command");
	// 	freeEnv(&newEnv, 1);
	// }
	// else if (child_pid == 0) {
	// 	environ = newEnv;
	// 	execvp(args[0], args);
	// }
	// else wait(NULL);

	freeEnv(&newEnv, 0);
}

void execCommand(char ***newEnv, int argc, char *argv[], int cmdIndex) {
	char **args = calloc(argc - cmdIndex + 1, sizeof(char*));
	if (args == NULL)
		freeEnv(newEnv, 1);

	int offset = cmdIndex;
	while (argv[cmdIndex] != NULL) {
		args[cmdIndex - offset] = argv[cmdIndex];
		cmdIndex++;
	}

	pid_t child_pid = fork();
	if (child_pid < 0) {
		freeEnv(newEnv, 0);
		perror("Error from calling command");
		exit(EXIT_FAILURE);
	}
	else if (child_pid == 0) {
		environ = *newEnv;
		execvp(args[0], args);
	}
	else wait(NULL);
}


void freeEnv(char ***newEnv, int err) {
	for (int i = 0; (*newEnv)[i] != NULL; i++) {
		free((*newEnv)[i]);
	}
	free(*newEnv);

	if (err == 1) {
		perror("Cannot allocate enough bytes for the environment");
		exit(EXIT_FAILURE);
	}
}

void modifyEnv(char ***newEnv, char *pair, char *pch) {
	int varLen = pch - pair + 2;
	char *varName = calloc(varLen, sizeof(char));
	strncpy(varName, pair, varLen-1);
	int pairAllocLen = strlen(pair)+1;

	int index;
	for (index = 0; (*newEnv)[index] != NULL; index++) {
		if (strstr((*newEnv)[index], varName) != NULL) {
			char *tmp_realloc = realloc((*newEnv)[index], pairAllocLen);
			if (tmp_realloc == NULL) 
				freeEnv(newEnv, 1);

			(*newEnv)[index] = tmp_realloc;
			strcpy((*newEnv)[index], pair);
			free(varName);
			return;
		}
	}
	free(varName);

	char **tmp_realloc = realloc(*newEnv, (index+2) * sizeof(char*));
	if (tmp_realloc == NULL) 
		freeEnv(newEnv, 1);

	*newEnv = tmp_realloc;
	(*newEnv)[index] = calloc(pairAllocLen, sizeof(char));
	if ((*newEnv)[index] == NULL) 
		freeEnv(newEnv, 1);

	strcpy((*newEnv)[index], pair);
	(*newEnv)[index+1] = NULL;
}

void copyEnv(char ***newEnv) {
	int i = 0;
	while (environ[i] != NULL) i++;
	*newEnv = realloc(*newEnv, (i+1) * sizeof(char*));

	if (*newEnv == NULL) 
		freeEnv(newEnv, 1);

	(*newEnv)[i] = NULL;

	for (i = 0; environ[i] != NULL; i++) {
		size_t len = strlen(environ[i]) + 1;
		(*newEnv)[i] = calloc(len, sizeof(char));
		if ((*newEnv)[i] == NULL) 
			freeEnv(newEnv, 1);

		strcpy((*newEnv)[i], environ[i]);
	}
}

void createEnv(char ***newEnv, int pairIndex, char *argv[]) {
	*newEnv = realloc(*newEnv, (pairIndex-1) * sizeof(char*));
	if (*newEnv == NULL)
		freeEnv(newEnv, 1);

	(*newEnv)[pairIndex-2] = NULL;

	for (int i = 2; i < pairIndex; i++) {
		(*newEnv)[i-2] = calloc(strlen(argv[i])+1, sizeof(char));

		if ((*newEnv)[i-2] == NULL)
			freeEnv(newEnv, 1);

		strcpy((*newEnv)[i-2], argv[i]);

	}
}

void displayEnv(char ***newEnv) {
	for (int i = 0; (*newEnv)[i] != NULL; i++)
		puts((*newEnv)[i]);
}


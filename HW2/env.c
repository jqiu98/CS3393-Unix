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
	int index = 1; // Index for argv
	bool modify = true; // Flag indicating if we are modifying the environment

	if (argc == 1) { // No optional input, just display environ
		displayEnv(&environ);
		return 0;
	}
	char **newEnv = calloc(1, sizeof(char*)); // Our environment 
	if (newEnv == NULL) freeEnv(&newEnv, 1);

	if (strcmp(argv[index],"-i") == 0) { // option -i used, not modifying
		modify = false; 
		index++;
	}
	else copyEnv(&newEnv); // we are modifying so copy environment onto heap

	while (argv[index] != NULL) {
		// If there's a '='' in the string then it's a key:value pair
		char *pch = strchr(argv[index], '=');
		if (pch != NULL) {
			if (modify) modifyEnv(&newEnv, argv[index], pch);
			index++;
		}
		else break; //
	}

	if (!modify && index != 2) createEnv(&newEnv, index, argv); // pairs entered after -i
	if (argv[index] == NULL) displayEnv(&newEnv); // No command entered

	execCommand(&newEnv, argc, argv, index); // execute command if there are
	freeEnv(&newEnv, 0); // free the heap
}


void freeEnv(char ***newEnv, int err) {
	// Free the nested pointers first
	for (int i = 0; (*newEnv)[i] != NULL; i++)
		free((*newEnv)[i]);
	free(*newEnv);

	if (err == 1) {
		perror("Cannot allocate enough bytes for the environment");
		exit(EXIT_FAILURE);
	}
}

void modifyEnv(char ***newEnv, char *pair, char *pch) {
	// Get the substring of the key:value pair up to & including the =
	int varLen = pch - pair + 2;
	char *varName = calloc(varLen, sizeof(char));
	strncpy(varName, pair, varLen-1);

	int pairAllocLen = strlen(pair)+1; //Alloc enough for the null terminator
	int index;

	// Loop through the environment to find a matching key,
	// replace that pair with the modified pair if found
	for (index = 0; (*newEnv)[index] != NULL; index++) {
		if (strstr((*newEnv)[index], varName) != NULL) {
			char *tmp_realloc = realloc((*newEnv)[index], pairAllocLen);
			if (tmp_realloc == NULL) freeEnv(newEnv, 1);

			(*newEnv)[index] = tmp_realloc;
			strcpy((*newEnv)[index], pair);
			free(varName);
			return;
		}
	}
	free(varName);

	// No matching key found, so realloc with +1 more space
	// and append the new pair into the environment
	char **tmp_realloc = realloc(*newEnv, (index+2) * sizeof(char*));
	if (tmp_realloc == NULL) freeEnv(newEnv, 1);

	*newEnv = tmp_realloc;
	(*newEnv)[index] = calloc(pairAllocLen, sizeof(char));
	if ((*newEnv)[index] == NULL) freeEnv(newEnv, 1);

	strcpy((*newEnv)[index], pair);
	(*newEnv)[index+1] = NULL;
}

void copyEnv(char ***newEnv) {
	// Find the size of environ and alloc just enough size to copy
	int i = 0;
	while (environ[i] != NULL) i++;
	*newEnv = realloc(*newEnv, (i+1) * sizeof(char*));
	if (*newEnv == NULL) freeEnv(newEnv, 1);

	(*newEnv)[i] = NULL; // Set the end null terminator since realloc doesn't zero

	// Loop through environ and copy over to the heap
	for (i = 0; environ[i] != NULL; i++) {
		size_t len = strlen(environ[i]) + 1;
		(*newEnv)[i] = calloc(len, sizeof(char));
		if ((*newEnv)[i] == NULL) freeEnv(newEnv, 1);

		strcpy((*newEnv)[i], environ[i]);
	}
}

void createEnv(char ***newEnv, int pairIndex, char *argv[]) {
	// Create an environment big enough for the key:value pairs entered
	*newEnv = realloc(*newEnv, (pairIndex-1) * sizeof(char*));
	if (*newEnv == NULL) freeEnv(newEnv, 1);

	(*newEnv)[pairIndex-2] = NULL; // Set the end null terminator since realloc doesn't zero

	// Copy over from argv into the new environment
	for (int i = 2; i < pairIndex; i++) {
		(*newEnv)[i-2] = calloc(strlen(argv[i])+1, sizeof(char));

		if ((*newEnv)[i-2] == NULL) freeEnv(newEnv, 1);

		strcpy((*newEnv)[i-2], argv[i]);

	}
}

void displayEnv(char ***newEnv) {
	// Loop through environment and output on display
	for (int i = 0; (*newEnv)[i] != NULL; i++)
		puts((*newEnv)[i]);
}


void execCommand(char ***newEnv, int argc, char *argv[], int cmdIndex) {
	// Create array* for the command and it's arguments from argv
	char **args = calloc(argc - cmdIndex + 1, sizeof(char*));
	if (args == NULL) freeEnv(newEnv, 1);

	// Copy over any data pertaining to command
	int offset = cmdIndex;
	while (argv[cmdIndex] != NULL) {
		args[cmdIndex - offset] = argv[cmdIndex];
		cmdIndex++;
	}

	// Fork so the child can call exec on the command
	// while parent is still maintained
	pid_t child_pid = fork();
	if (child_pid < 0) { // In parent, error with child
		freeEnv(newEnv, 0);
		perror("Error from calling command");
		exit(EXIT_FAILURE);
	}
	else if (child_pid == 0) { // In child
		// Replace the environment with the new/modified one
		environ = *newEnv;
		execvp(args[0], args);
	}
	else wait(NULL); // In parent, wait for child to finish
}

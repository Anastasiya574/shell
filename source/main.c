// Copyright 2020 Anastasiya Park
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

char *get_word(char *end) {
	if ((*end) == '\n') {
		return NULL;
	}
	char symbol;
	char *word = (char*)malloc(sizeof(char));
	int i = 0;
	symbol = getchar();
	while ((symbol != ' ') && (symbol != '\t') && (symbol != '\n')) {
		word = realloc(word, i + 2);
		if (!word) {
			printf("Word allocation error");
			exit(1);
		}
		word[i] = symbol;
		i++;
		symbol = getchar();
	}
	word[i] = '\0';
	(*end) = symbol;
	return word;
}

char **get_list() {
	int i = 0;
	char **list = (char**)malloc(sizeof(char*));
	char *word;
	char end = 0;
	word = get_word(&end);
	while (word) {
		list = realloc(list, (i + 2) * sizeof(char*));
		if (!list){
			printf("List allocation error");
			exit(1);
		}
		list[i] = word;
		i++;
		word = get_word(&end);
	}
	list[i] = NULL;
	return list;
}

void clear(char **list) {
    int i = 0;
    while (list[i] != NULL) {
        free(list[i]);
        i++;
    }
		free(list[i]);
    free(list);
}


int is_end(char **list) {

    if ((strcmp(list[0], "exit") == 0) || (strcmp(list[0], "quit") == 0)) {
        return 1;
    } else {
        return 0;
    }
}



int redirect(char **cmd){
    int fd;
    int i = 0;
    int check = 0;
    pid_t pid;
    if ((pid = fork()) == 0) {
        while (cmd[i] != NULL) {
            if (cmd[i][0] == '>') {
                //printf("finded\n" );
                if (cmd[i + 1]!= NULL) {
                    fd = open(cmd[i + 1], O_WRONLY | O_CREAT | O_TRUNC,
                                          S_IRUSR | S_IWUSR);
                    check = 1;
                }
                break;
            }
            if (cmd[i][0] == '<') {
                if (cmd[i + 1]!= NULL) {
                    fd = open(cmd[i + 1], O_RDONLY);
                    check = 2;
                }
                break;
            }
            i++;
        }
        if (fd == -1) {
            perror("file is not open");
            return 1;
        }
        if (check == 1) {
            dup2(fd, 1);
            cmd[i] = 0;
            free(cmd[i + 1]);
        }
        if (check == 2) {
            dup2(fd, 0);
            cmd[i] = 0;
            free(cmd[i + 1]);
        }
        execvp(cmd[0], cmd);
    }
    wait(0);
    return 0;
}



int main(int argc, char** argv) {
    char **cmd = NULL;
		cmd = get_list();
		while(is_end(cmd) == 0) {
			redirect(cmd);
			clear(cmd);
			cmd = get_list();
		}
		clear(cmd);
    return 0;
}

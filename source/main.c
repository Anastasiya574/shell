// Copyright 2020 Anastasiya Park
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

char *get_word(char *end);
char **get_list();
void clear(char **list);
int is_end(char **list);
int is_delimiter(char **list, int **head);
int redirect(char **cmd, int head);
void run(char **list, int *head);

int main() {
    char **cmd = get_list();
    int *head;
    head = malloc(1 * sizeof(int));
    head[0] = 0;
    while(is_end(cmd) == 0) {
        run(cmd, &head);
        clear(cmd);
        free(head);
        head = malloc(1 * sizeof(int));
        head[0] = 0;
        cmd = get_list();
    }
    clear(cmd);
    free(head);
    return 0;
}

char *get_word(char *end) {
	if ((*end) == '\n') {
		return NULL;
	}
	char symbol;
	char *word = (char*)malloc(sizeof(char));
	int i = 0;
	symbol = getchar();
	while ((symbol != ' ') && (symbol != '\n')) {
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

int is_delimiter(char **list, int **head) {
    int i = 0;
    int counter = 0;
    while (list[i]) {
        if (strcmp(list[i], "|") == 0) {
            free(list[i]);
            list[i] = NULL;
            counter++;
            *head = realloc(*head, (counter + 1) * sizeof(int));
            (*head)[counter] = i + 1;
        }
        i++;
    }
    return counter;
}

int redirect(char **cmd, int head) {
    int fd_in = 0, fd_out = 0;
    int i = head;
    while (cmd[i] != NULL) {
        if (cmd[i][0] == '>') {
            fd_out = open(cmd[i + 1],
                O_WRONLY | O_CREAT | O_TRUNC ,
                S_IRUSR | S_IWUSR);
            dup2(fd_out, 1);
            free(cmd[i]);
            cmd[i] = NULL;
            free(cmd[i + 1]);
            cmd[i + 1] = NULL;
            i++;
            close(fd_out);
        }
        if (cmd[i][0] == '<') {
            fd_in =  open(cmd[i + 1], O_RDONLY);
            dup2(fd_in, 0);
            free(cmd[i]);
            cmd[i] = NULL;
            free(cmd[i + 1]);
            cmd[i + 1] = NULL;
            i++;
            close(fd_in);
        }
        i++;
    }
    return 0;
}


void run(char **list, int *head) {
        int pipe_in_phone = 0;
        if (list[0] != NULL) {
            pipe_in_phone = is_delimiter(list, &head);
            int (*fd)[2] = malloc((pipe_in_phone + 1) * sizeof(int[2]));
            for (int i = 0; i <= pipe_in_phone; i++) {
                pipe(fd[i]);
            }
            if (fork() == 0) {
                if (pipe_in_phone != 0) {
                     dup2(fd[0][1], 1);
                }
                close(fd[0][1]);
                close(fd[0][0]);

                redirect(list, head[0]);
                if (execvp(list[head[0]], list+head[0]) < 0) {
                     perror("exec failed");
                }
            for (int i = 1; i <= pipe_in_phone; i++) {
                if (fork() == 0) {
                    dup2(fd[i - 1][0], 0);
                    close(fd[i - 1][0]);
                    close(fd[i - 1][1]);

                    if (i != pipe_in_phone) {
                        dup2(fd[i][1], 1);
                    }
                    close(fd[i][1]);
                    close(fd[i][0]);

                    redirect(list, head[i]);
                if (execvp(list[head[i]], list+head[i]) < 0) {
                     perror("exec failed");
                } 
                } else {
                    close(fd[i - 1][1]);
                    close(fd[i - 1][0]);
                }
            }
            close(fd[pipe_in_phone][1]);
            close(fd[pipe_in_phone][0]);

            for (int i = 0; i <= pipe_in_phone; i++) {
                wait(NULL);
            }
            free(fd);
        }
    return;
    }
}

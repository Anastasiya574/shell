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
void check_and_run(char **list);
void conveyor_and(char **list, int fd_in, int fd_out);
void run(char **list, int fd_in, int fd_out, int delimiter_position, int and_position, int bg_mode);
void exec_classic(char ** list, int fd_in, int fd_out, int bg_mode);
void make_3star(char **list, char ***A, char ***B, int delimiter_position);
void conveyor_for_two(char **list, char **A, char **B, int fd_in, int fd_out);
void handler(int signo);


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

void check_and_run(char **list) {
	int i = 0;
        int fd_in = 0, fd_out = 1;
        int and_position = 0, delimiter_position = 0, bg_mode = 0;
	while (list[i] != NULL) {
                //redirect_in(list, fd_in);
                //redirect_out(list, fd_out);
		if (strcmp(list[i], "<") == 0) {
			fd_in = open(list[i + 1], O_RDONLY);
			if (fd_in < 0) {
				perror("Open failed");
				return;
			}
			break;
		}
		else if (strcmp(list[i], ">") == 0) {
			fd_out = open(list[i + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
			if (fd_out < 0) {
				perror("Open failed");
				return;
			}
			break;
		}
		if (strcmp(list[i], "|") == 0) {
			delimiter_position = i;
		}
		if (strcmp(list[i], "&") == 0) {
			bg_mode = 1;
			return;;
		}
		if (strcmp(list[i], "&&") == 0) {
			and_position = i;
			break;
		}
		i++;
	}
	list = realloc(list, (i + 1) * sizeof(char*));
	list[i] = NULL;
	run(list, fd_in, fd_out, delimiter_position, and_position, bg_mode);
        return;
}

void conveyor_and(char **list, int fd_in, int fd_out) {
	char **current;
	int i = 0, length = 0, counter = 0, j;
	while (1) {
		if (list[i] == NULL || strcmp(list[i], "&&") == 0) {
			length = i - counter + 1;
			current = (char **)malloc((length) * sizeof(char*));
			for (j = 0; j < length - 1; j++, counter++) {
				current[j] = list[counter];
			}
			current[j] = NULL;
			int wstatus;
			if ((fork()) > 0) {
				wait(&wstatus);
			} else {
				if (fd_out != 1) {
					dup2(fd_out, 1);
				}
				if (fd_in != 0) {
					dup2(fd_in, 0);
				}
				if (execvp(current[0], current) < 0) {
					perror("exec failed");
					_exit(1);
				}
			}
			printf("%d\n", WEXITSTATUS(wstatus));
			free(current);
			if (list[i] == NULL) {
				break;
			}
			counter = i + 1;
		}
		i++;
	}
}

void run(char **list, int fd_in, int fd_out, int delimiter_position,
         int and_position, int bg_mode) {
	if (delimiter_position != 0) {
		char **A, **B;
		make_3star(list, &A, &B, delimiter_position);
		conveyor_for_two(list, A, B, fd_in, fd_out);
	} else if (and_position != 0) {
		conveyor_and(list, fd_in, fd_out);
	} else {
		exec_classic(list, fd_in, fd_out, bg_mode);
	}
	if (fd_out != 1) {
		close(fd_out);
	}
	if (fd_in != 0) {
		close(fd_in);
	}
        return;
}



void exec_classic(char ** list, int fd_in, int fd_out, int bg_mode) {
	if (fork() > 0) {
		if (bg_mode == 0) {
			wait(NULL);
		}
	} else {
		if (fd_out != 1) {
			dup2(fd_out, 1);
		}
		if (fd_in != 0) {
			dup2(fd_in, 0);
		}
		if (execvp(list[0], list) < 0) {
			perror("exec failed");
			return;
		}
	}
        return;
}


void make_3star(char **list, char ***A, char ***B, int delimiter_position) {
	int i, k, j;
	(*A) = (char **)malloc((delimiter_position + 1) * sizeof(char*));
	for (i = 0; i < delimiter_position; i++) {
		(*A)[i] = list[i];
	}
	(*A)[i] = NULL;
	i = delimiter_position + 1;
	while (list[i] != NULL) {
		i++;
	}
	(*B) = (char **)malloc((i - delimiter_position) * sizeof(char*));
	for (k = 0, j = delimiter_position + 1; j < i; k++, j++) {
		(*B)[k] = list[j];
	}
	(*B)[k] = NULL;
        return;
}

void conveyor_for_two(char **list, char **A, char **B, int fd_in, int fd_out) {
	int fd[2];
	pipe(fd);
	if (fork() == 0) {
		if (fd_in != 0) {
			dup2(fd_in, 0);
		}
		close(fd[0]);
		dup2(fd[1], 1);
		execvp(A[0], A);
		_exit(1);
	}
	if (fork() == 0) {
		if (fd_out != 1) {
			dup2(fd_out, 1);
		}
		close(fd[1]);
		dup2(fd[0], 0);
		execvp(B[0], B);
		_exit(1);
	}
	close(fd[0]);
	close(fd[1]);
	wait(NULL);
	wait(NULL);
	return;
}
/*
void handler(int signo) {
	pid_t pid;
	if (pid != 1) {
		kill(pid, SIGINT);
	}
}


int main(void) {
	char **cmd;
        //signal(SIGINT, handler);
	while(1) {
		cmd = get_list();
		if (is_end(cmd) == 1) {
			clear(cmd);
			break;
		}
		check_and_run(cmd);
		clear(cmd);
	}
	return 0;
}

*/
int main(int argc, char** argv) {
    char **cmd = NULL;
    cmd = get_list();
    while (is_end(cmd) == 0) {
	check_and_run(cmd);
	clear(cmd);
	cmd = get_list();
    }
    clear(cmd);
    return 0;
}

// Copyright 2020 Anastasiya Park
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <err.h>

char *get_word(char *end) {
    char ch;
    char *word = (char *)malloc(sizeof(char));
    int i = 0;
    ch = getchar();
    while ((ch!= ' ') && (ch != '\t') && (ch != '\n')) {
        word = realloc(word, i + 1);
        if (!word) {
            printf("Word allocation error");
            exit(1);
        }
        word[i] = ch;
        i++;
        ch = getchar();
    }
    word[i] = '\0';
    (*end) = ch;
    return word;
}

char **get_list() {
    int i = 0;
    char **list = (char**)malloc(sizeof(char*));
    char *word;
    char end = 0;
    word = get_word(&end);
    while (end != '\n') {
        list = realloc(list, (i + 2) * sizeof(char*));
        list[i] = word;
        i++;
        word = get_word(&end);
    }
    list[i] = NULL;
    return list;
}

void clear(char **list) {
    int i = 0;
    if (list == NULL) {
        return;
    }
    while (list[i] != NULL) {
        free(list[i]);
        i++;
    }
    free(list[i]);
    free(list);
}

int is_end_of(char **list) {
    if ((strcmp(list[0], "exit") == 0) || (strcmp(list[0], "quit") == 0)) {
        return 1;
    } else {
        return 0;
    }
}

void redirect(char **list) {
    int i = 0;
    int fd_in = 0, fd_out = 1;
    while (list[i]) {
        if (strcmp(list[i], "<") == 0) {
            fd_in = open(list[i + 1], O_RDONLY);
            if (fd_in < 0) {
                perror("'<'(open) failed");
                exit(1);
            }
            break;
        }
        else if (strcmp(list[i], ">") == 0) {
            fd_out = open(list[i + 1], O_WRONLY | O_CREAT | O_TRUNC,
                                            0644);
            if (fd_out < 0) {
                perror("'>'(open) failed");
                exit(1);
            }
            break;
        }
        i++;
    }

}


int main(int argc, char** argv) {
    char **cmd = NULL;
    cmd = get_list();
	/*if(cmd!= NULL) {
		printf("isn't NULL!!!!\n");
    }
    while (!is_end_of(cmd)) {
        pid_t pid;
        if ((pid = fork()) == 0) {
            if(execvp(cmd[0], cmd) < 0){
                perror ("fail");
			}
        } else {
            wait(0);
        }
        clear(cmd);
        cmd = get_list();
    }*/
    redirect(cmd);
    if (fork() > 0) {
         wait(NULL);
    } else {
        if (execvp(cmd[0], cmd) < 0) {
                perror ("exec failed");
                return 1;
        }
    }
    clear(cmd);
    return 0;
}

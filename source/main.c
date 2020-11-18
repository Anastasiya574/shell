// Copyright 2020 Anastasiya Park
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

void clear(char **list) {
  int i = 0;
  if(list == NULL) return;
  while (list[i]!= NULL) {
    free(list[i]);
    i++;
  }
  free(list);
}

int is_end(char **list) {
    if ((strcmp(list[0], "exit") == 0) || (strcmp(list[0], "quit") == 0)) {
        return 1;
    } else {
        return 0;
    }
}
// new clear func to kill this segfault and run prog
void free_listing(char ***list, int wd_cnt) {
    for (int i = 0; i < wd_cnt; i++) {
      free(list[i]);
    }
  free(list);
}

void freelist_pos(char **list, int pos) {
  int i = 0;
  if (list == NULL) return;
  while (list[i]!= NULL) {
    free(list[i]);
    i++;
  }
  i = pos;
  while (list[i]!= NULL) {
    free(list[i]);
    i++;
  }
  free(list);
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

char ***make_3star(char **list) {
  int delimiter_cnt = 0;
  int word_cnt = 0;
  char ***array = NULL;
  array = malloc(100 * sizeof(char**));
  array[delimiter_cnt] = malloc(100 * sizeof(char*));
  for (int i = 0; list[i]; i++) {
      if ((strcmp(list[i], "|")) == 0) {
          array[delimiter_cnt][word_cnt] = NULL;
          delimiter_cnt++;
          word_cnt = 0;
          array[delimiter_cnt] = malloc(100 * sizeof(char*));
      }
      else {
        array[delimiter_cnt][word_cnt] = list[i];
        word_cnt++;
      }
  }
  if (word_cnt!= 0) {
      array[delimiter_cnt][word_cnt] = NULL;
  }
  return array;
}

int delimiter_counter(char **list) {
  int cnt = 0;
  int i = 0;
  while(list[i]!= NULL) {
    if(list[i][0] == '|') {
      cnt++;
    }
    i++;
  }
  return cnt;
}

void pipe_conveyor(char **list, int n) {
  char ***array = NULL;
  array = make_3star(list);
  int pipe_fd[100][2] ,pid;
  int i = 0;
  for(i = 0; i <= n; i++) {
    if(i!= n) pipe(pipe_fd[i]);

    if((pid = fork()) == 0) {
      if(i!= 0) {
        dup2 (pipe_fd[i - 1][0] , 0);
        close (pipe_fd[i - 1][0]);
        close (pipe_fd[i - 1][1]);
      }
      if (i!= n) {
        dup2 (pipe_fd[i][1] , 1);
        close (pipe_fd[i][1]);
        close (pipe_fd[i][0]);
      }
      if((execvp (array[i][0], array[i])) < 0) {
        perror("error exec2"); exit(1); 
      }
      exit(0);
    }
      else {
        if(i!= 0) {
          close (pipe_fd[i - 1][1]);
          close (pipe_fd[i - 1][0]);
        }
      wait(NULL);
    }
  }
  if (n > 0) {
    close (pipe_fd[n - 1][1]);
    close (pipe_fd[n - 1][0]);
  }
  free_listing(array, n + 1);
}

int if_redirect(char **list) {
  int check;
  int i = 0;
  while(list[i]!= NULL) {
    if(list[i][0] == '>') {
      if(list[i + 1]!= NULL) {
        check = 1;
      }
      break;
    }
    if(list[i][0] == '<') {
      if(list[i + 1]!= NULL) {
        check = 2;
      }
      break;
    }
    i++;
  }
  return check;
}

int fileopen (char **list, int i) {
  int fd;
  if (if_redirect(list) == 1){
    fd = open(list[i + 1], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  }
  if (if_redirect(list) == 2){
    fd = open(list[i + 1], O_RDONLY );
  }
  return fd;
}

void redirect_exec(char **list) {
  pid_t pid;
  int fd;
  int i = 0;
  if ((pid = fork()) == 0) {
    if (if_redirect(list) == 1) {
          while(list[i][0]!= '>') i++;
          fd = fileopen(list, i);
          dup2(fd, 1);
          list[i] = 0;
          free(list[i + 1]);
        }
    if (if_redirect(list) == 2) {
          while (list[i][0]!= '<') i++;
          fd = fileopen(list, i);
          dup2 (fd, 0);
          list[i] = 0;
          free (list[i + 1]);
        }
    if (fd == -1) {
      perror("file is not open for read ore write");
    }
    if((execvp (list[0], list)) < 0) { 
      perror("error"); exit(0);
    }
  }
  wait(0);
}

int check (char **list) {
  int ch = 0;
  int i = 0;
  while (list[i]!= NULL) {
    if (list[i][0] == '|') {
      ch = 1;
    }
    i++;
  }
  return ch;
}
////////////no need because of pipe_conveyor
int conveyor_for_two(char **list) {
    int i = 0, cnt = 0;
      while(list[cnt]!= NULL) cnt++;
      while(list[i][0]!= '|') i++;
      list[i] = NULL;
      int pos = i + 1;
      int fd[2];
      pipe(fd);
      if(fork() == 0) {
        dup2(fd[1], 1);
        close(fd[0]);
        close(fd[1]);
        execvp(list[0], list);
      }
      if (fork() == 0) {
        dup2(fd[0], 0);
        close(fd[0]);
        close(fd[1]);
        execvp(list[pos], list + pos);
      }
      close(fd[0]);
      close(fd[1]);
      wait(NULL);
      wait(NULL);
      return pos;
}
////////////////////
void change_dir(char **list) {
    char *home = getenv("HOME");
    if (strcmp(list[0], "cd") == 0) {
        if (list[1] == NULL || (strcmp(list[1], "~") == 0)) {
            chdir(home);
        } else {
            chdir(list[1]);
        }
    }
}

int if_phone(char **list) {
  int i = 0;
  int check = 0;
  while (list[i]!= NULL) i++;
  if ((strcmp(list[i - 1] , "&")) == 0) {
    check = 1;
  }
  return check;
}

int phone_pids[100] , pid_cnt = 0;

void bg_exec(char **list) {
  pid_t pid;
  int dev;
  if ((pid = fork()) == 0) {
    int i = 0;
    while (list[i]!= NULL) i++;
    close(0);
    close(1);
    dev = open("/dev/null", O_RDWR);
    dup(dev);
    list[i - 1] = NULL;
    free(list[i]);
    free(list[i - 1]);
    if ((execvp(list[0], list)) < 0) {
      perror("err with phone exec");
      exit(1);
    }
    exit(1);
  }
  phone_pids[pid_cnt] = pid;
  pid_cnt++;
  printf("phone pid:%d\n", phone_pids[pid_cnt]);
}

int main() {
  char **list = NULL;
  int cnt = 0;
  int check_phone;
  list = get_list();
  while (is_end(list) == 0) {
        change_dir(list);
        if ((check_phone = if_phone(list)) == 1) {
          bg_exec(list);
          clear(list);
        } else if ((cnt = delimiter_counter(list)) > 0) {
          pipe_conveyor(list, cnt);
          clear(list);
        } else {
          redirect_exec(list);
          clear(list);
        }
        list = get_list();
  }
  clear(list);
  int j = 0;
  while (j!= pid_cnt) {
    waitpid(phone_pids[pid_cnt], NULL , 0);
    j++;
  }
  return 0;
}

/*
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
					return;
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
} */

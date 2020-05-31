/**
 * Author: Topiltzin Hernández Mares
 */

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFF_MAX 1024
#define TOK_MAX 60
#define ALLOCATION_ERROR "tshell: allocation error\n"
#define MAX_LINE_LEN 512

void shell_loop();
int file_loop(char**);
int single_command(char*);
int many_commands(char*);
char **get_args(char*);
char *get_command();
void check_for_cd(char**);
int exec_command(char**);

int main(int argc, char *argv[]) {
  if (argc > 2) {
    perror(argv[0]);
    return -1;
  }

  if (argc == 1) {
    shell_loop();
  }

  if (argc == 2) {
    file_loop(argv);
  }

  return 0;
}

void shell_loop() {
  char *line;
  char cwd[PATH_MAX];
  int status = 1;
  int has_quit = 0;

  while (status && !has_quit) {
    if ( getcwd(cwd, sizeof(cwd)) != NULL ) {
      printf("\n%s\n> $ ", cwd);
    }

    line = get_command(NULL);

    if (strstr(line, "quit") != NULL) {
      has_quit = 1;
    }

    if (strstr(line, ";") != NULL) {
      status = many_commands(line);
    } else {
      if (strlen(line) != 0) {
        if (!has_quit) {
          status = single_command(line);
        }
      }
    }
    free(line);
  }
}

int file_loop(char *argv[]) {
  int fd;
  char line[MAX_LINE_LEN];
  int has_quit = 0;

  if ( (fd = open(argv[1], O_RDONLY)) < 0 ) {
    perror(argv[0]);
    return -1;
  }

  FILE *file = fdopen(fd, "r");

  while (fgets(line, MAX_LINE_LEN, file) && !has_quit) {
    if (strstr(line, "quit") != NULL) {
      has_quit = 1;
    }
    if (strstr(line, ";") != NULL) {
      many_commands(line);
    } else {
      if (strlen(line) != 0 && !has_quit) {
        single_command(line);
      }
    }
  }

  close(fd);

  return 0;
}

int single_command(char *line) {
  int final_status;
  char **argv;
  int pid;


  argv = get_args(line);
  check_for_cd(argv);
  pid = fork();

  if (pid == 0) {
    int return_status = exec_command(argv);
    exit(return_status);
  } else if (pid < 0) {
    perror("tsh");
    exit(-1);
  } else {
    do {
      waitpid(pid, &final_status, WUNTRACED);
    } while (!WIFEXITED(final_status) && !WIFSIGNALED(final_status));
    free(argv);
    return 1;
  }
}

int many_commands(char *line) {
  int status = 1;
  int command_count = 0;
  int pid;
  char *command;
  char **argv;

  command = strtok(line, ";");
  while(command != NULL) {
    pid = fork();

    if (strstr(command, "quit") != NULL) {
      exit(0);
    }

    if (pid == 0) {
      argv = get_args(command);

      int return_status = exec_command(argv);
      free(argv);
      exit(return_status);
    } else {
      command_count++;
      command = strtok(NULL, ";");
    }
  }

  while (command_count-- > 0) {
    wait(NULL);
  }

  return status;
}

char *get_command() {
  int size = BUFF_MAX;
  char *buffer = malloc(sizeof(char) * size);
  int character;
  int position = 0;

  if (!buffer) {
    fprintf(stderr, ALLOCATION_ERROR);
    exit(-1);
  }

  while(1) {
    character = getchar();

    if (position >= BUFF_MAX) {
      size += BUFF_MAX;
      buffer = realloc(buffer, size);

      if (!buffer) {
        fprintf(stderr, ALLOCATION_ERROR);
        exit(-1);
      }
    }

    if (character == '\n' || character == EOF) {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = character;
    }
    position++;
  }
}

char **get_args(char *command) {
  int tok_size = TOK_MAX;
  char **tokens = malloc(sizeof(char*) * tok_size);
  char *token;
  int position = 0;

  if (!tokens) {
    fprintf(stderr, ALLOCATION_ERROR);
    exit(-1);
  }

  token = strtok(command, " ");
  while(token != NULL) {
    if (position >= tok_size) {
      tok_size += TOK_MAX;
      tokens = realloc(tokens, sizeof(char*) * tok_size);

      if (!tokens) {
        fprintf(stderr, ALLOCATION_ERROR);
        exit(-1);
      }
    }

    tokens[position] = token;
    position++;

    token = strtok(NULL, " ");
  }
  tokens[position] = NULL;
  return tokens;
}

void check_for_cd(char *argv[]) {
  if (strcmp(argv[0], "cd") == 0) {
    chdir(argv[1]);
  }
}

int exec_command(char *argv[]) {
  if (execvp(argv[0], argv) == -1) {
    perror("tsh");
    return -1;
  }

  return 1;
}

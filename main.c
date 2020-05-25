/**
 * Author: Topiltzin Hernández Mares
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#define BUFF_MAX 1024
#define TOK_MAX 60
#define ALLOCATION_ERROR "tshell: allocation error\n"

void shell_loop();
char **get_args(char*);
char *get_command();
int exec_command(char**);

int main(int argc, char *argv[]) {

  if (argc == 1) {
    shell_loop();
  }

  return 0;
}

void shell_loop() {
  char *line;
  char **argv;
  char cwd[PATH_MAX];
  int status = 1;

  while (status) {
    if ( getcwd(cwd, sizeof(cwd)) != NULL ) {
      printf("\n%s\n> $ ", cwd);
    }

    line = get_command();
    argv = get_args(line);
    status = exec_command(argv);

    free(line);
    free(argv);
  }
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

int exec_command(char *argv[]) {
  int pid;
  int status;

  // Check for `cd`
  if (strcmp(argv[0], "cd") == 0) {
    chdir(argv[1]);
    return 1;
  }
  pid = fork();

  if (pid == 0) {
    // Child to execute the command
    if (execvp(argv[0], argv) == -1) {
      perror("tsh");
      exit(-1);
    }
    exit(1);
  } else if (pid < 0) {
    perror("tsh");
  } else {
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

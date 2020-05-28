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
int single_command(char*);
int many_commands(char*);
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

    if (strstr(line, ";") != NULL) {
      status = many_commands(line);
    } else {
      status = single_command(line);
    }
    free(line);
  }
}

int single_command(char *line) {
  int final_status = 1;
  char **argv;
  int pid;

  pid = fork();
  argv = get_args(line);

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
  int commands_count = 0;
  char *command, *cmd_cpy;
  char **argv;

  command = strtok(line, ";");
  while(command != NULL) {
    cmd_cpy = malloc(1 + strlen(command));
    strcpy(cmd_cpy, command);

    argv = get_args(cmd_cpy);
    printf("Command: %s\n", argv[0]);
//        status = exec_command(argv);

    command = strtok(NULL, ";");
    commands_count++;

    free(argv);
    free(cmd_cpy);
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

int exec_command(char *argv[]) {
  // Check for `cd`
  if (strcmp(argv[0], "cd") == 0) {
    chdir(argv[1]);
    return 1;
  }

  if (execvp(argv[0], argv) == -1) {
    perror("tsh");
    return -1;
  }

  return 1;
}

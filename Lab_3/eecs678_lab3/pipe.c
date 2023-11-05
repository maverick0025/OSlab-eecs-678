#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>

#define R_FILE "/proc/meminfo"
#define BSIZE 256 

int main()
{
  int status;
  pid_t pid_1, pid_2;
  int pipe_fd[2]; // File descriptors for the pipe

  /* Create a pipe before forking any children */
  if (pipe(pipe_fd) == -1) {
    perror("pipe");
    return EXIT_FAILURE;
  }

  pid_1 = fork();
  if (pid_1 == 0) { 
    /* process a (child 1) */ 

    close(pipe_fd[0]); // Close the read end of the pipe in child 1

    int rfd;
    size_t rsize;
    char buf[BSIZE];

    if ((rfd = open(R_FILE, O_RDONLY)) < 0) {
      fprintf(stderr, "\nError opening file: %s. ERROR#%d\n", R_FILE, errno);
      return EXIT_FAILURE;
    }

    /* read contents of file and write it out to the pipe */
    while ((rsize = read(rfd, buf, BSIZE)) > 0) {
      write(pipe_fd[1], buf, rsize); // Write to the pipe
    }

    close(rfd);
    close(pipe_fd[1]); // Close the write end of the pipe in child 1
    return 0; 
  } 

  pid_2 = fork();
  if (pid_2 == 0) {
    /* process b (child 2) */

    close(pipe_fd[1]); // Close the write end of the pipe in child 2

    size_t rsize;
    char buf[BSIZE];

    /* read from the pipe and process the data */
    while ((rsize = read(pipe_fd[0], buf, BSIZE)) > 0) {
      // Process data from the pipe (buf contains the data read from the pipe)
      // You can perform your desired operations here.
    }

    close(pipe_fd[0]); // Close the read end of the pipe in child 2
    return 0;
  }

  /* Parent process */
  close(pipe_fd[0]); // Close both ends of the pipe in the parent
  close(pipe_fd[1]);

  /* Wait for child processes to complete */
  waitpid(pid_1, &status, 0);
  waitpid(pid_2, &status, 0);

  return 0;
}


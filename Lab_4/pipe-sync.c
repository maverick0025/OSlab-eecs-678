/*
  pipe-sync.c: Use Pipe for Process Synchronization
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

// Question: Update this program to synchronize execution of the parent and
// child processes using pipes, so that we are guaranteed the following
// sequence of print messages:
// Child line 1
// Parent line 1
// Child line 2
// Parent line 2


int main()
{
  char *s, *d, buf[1024];
  int ret, stat;
  int fds[2];
  //int fdss[2];
  s  = "Use Pipe for Process Synchronization\n";
  //d  = "secondly\n";

   
  /* create pipe */

  pipe(fds);
  //pipe(fdss);

  ret = fork();
  if (ret == 0) {

    /* child process. */
    close(fds[0]);
    //close(fdss[0]);
    
    
    printf("Child line 1\n");
    write(fds[1], buf, strlen(s));
    close(fds[1]); 
    sleep(1);
    
    //write(fdss[1], buf, strlen(d));
   
    //close(fdss[1]);
    
    printf("Child line 2\n");
  } else {

    /* parent process */
    close(fds[1]);
    //close(fdss[1]);
    
    read(fds[0], s, strlen(s));
    printf("Parent line 1\n");
    close(fds[0]);
    sleep(2); 
    //read(fdss[0], d, strlen(d));
    //close(fdss[0]);
    printf("Parent line 2\n");
   
    wait(&stat);
    
      }
}

/*
  Pavorn Thongyoo 6480138
  Pachara Akkanwanich 6480125
  Panachai Kongja 6480068 
  Pornlapat Thammarattanapruk 6381437
*/
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#define FFNAME12 "/tmp/fifo1to2"
#define FFNAME21 "/tmp/fifo2to1"
#define MAX_RBUF 80

static pid_t child_pid;

static void sigterm_handler(int signum) {
    if (child_pid > 0) {
        kill(child_pid, SIGTERM);
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {

  int user = atoi(argv[1]);
  int fd_rdwr12, fd_rdwr21, pid, pnum, anum, bytes_read, bytes_written, c=0;
  
  if (access(FFNAME12, F_OK) == -1){
    // Create the FIFO for writing and reading from 1 to 2
    if (mkfifo(FFNAME12, 0666) == -1) {
        perror("mkfifo fails");
        exit(EXIT_FAILURE);
    }
  }
  
  if (access(FFNAME21, F_OK) == -1){
    // Create the FIFO for writing and reading from 2 to 1
    if (mkfifo(FFNAME21, 0666) == -1) {
        perror("mkfifo fails");
        exit(EXIT_FAILURE);
    }
  }

  // Open the FIFO for reading and writing from 1 to 2
  if ((fd_rdwr12 = open(FFNAME12, O_RDWR)) == -1) {
      perror("open fails");
      exit(EXIT_FAILURE);
  }

  // Open the FIFO for reading and writing from 2 to 1
  if ((fd_rdwr21 = open(FFNAME21, O_RDWR)) == -1) {
      perror("open fails");
      exit(EXIT_FAILURE);
  }
  
  // user 1
  if (user ==  1){
    char rbuf[MAX_RBUF] = ""; // buffer to store string from terminal
    
    pid_t pid = fork();
    child_pid = pid;
    // fork fail
    if (pid < 0){
      fprintf(stderr, "Fork failed!\n");
      exit(0);
    }
    // child process
    else if (pid == 0){
      char cbuffer[BUFSIZ + 1];
      memset(cbuffer, '\0', sizeof(cbuffer));

      while(1){ 
        // read from fifo2to1
        if ((bytes_read = read(fd_rdwr21, cbuffer, BUFSIZ)) > 0){
          // if 'end chat', send signal to parent
          if (strncmp(cbuffer,"end chat",8)==0){
            kill(getppid(), SIGTERM);
            exit(0);
          } 
          printf("%s", cbuffer);
          memset(cbuffer, '\0', sizeof(cbuffer));
        }
      }
    } // end child
    // parent process
    else {
      while(1){
        // get input and put it into buffer
        if (fgets(rbuf, sizeof(rbuf), stdin) == NULL) {
          fprintf(stderr, "Error reading input from stdin\n");
          exit(EXIT_FAILURE);
        }
        // write into fifo1to2
        int bytes_written = write(fd_rdwr12, rbuf, strlen(rbuf));
        // if 'end chat', receive signal from child and use the function to terminate child
        if (strncmp(rbuf, "end chat", 8) == 0) {
          signal(SIGTERM, sigterm_handler);
          exit(0);
        }
        if (bytes_written == -1) {
          perror("write fails");
          exit(EXIT_FAILURE);
        }
        memset(rbuf, '\0', sizeof(rbuf));
      }
    } // end parent
    
  } // end user 1
  // user 2
  else if(user == 2){
    char rbuf[MAX_RBUF] = ""; // buffer to store string from terminal
    
    pid_t pid = fork();
    child_pid = pid;
    // fork fail
    if (pid < 0){
      fprintf(stderr, "Fork failed!\n");
      exit(0);
    }
    // child process
    else if (pid == 0){
      char cbuffer[BUFSIZ + 1];
      memset(cbuffer, '\0', sizeof(cbuffer));
      while(1){ 
        // read from fifo1to2
        if ((bytes_read = read(fd_rdwr12, cbuffer, BUFSIZ)) > 0){
           // if 'end chat', send signal to parent
          if (strncmp(cbuffer,"end chat",8)==0){
            kill(getppid(), SIGTERM);
            exit(0);
          }
          printf("%s", cbuffer);
          memset(cbuffer, '\0', sizeof(cbuffer));
        }
      }
    } // end child
    // parent process
    else {
      while(1){
        // get input and put it into buffer
        if (fgets(rbuf, sizeof(rbuf), stdin) == NULL) {
          fprintf(stderr, "Error reading input from stdin\n");
          exit(EXIT_FAILURE);
        }
        // write into fifo2to1
        int bytes_written = write(fd_rdwr21, rbuf, strlen(rbuf));
        if (bytes_written == -1) {
          perror("write fails");
          exit(EXIT_FAILURE);
        }
        // if 'end chat', receive signal from child and use the function to terminate child
        if (strncmp(rbuf, "end chat", 8) == 0) {
          signal(SIGTERM, sigterm_handler);
          exit(0);
        }
        memset(rbuf, '\0', sizeof(rbuf));
      }
    } // end parent
    
  } // end user 2
  else{
    fprintf(stderr, "Error reading input.\n");
    exit(EXIT_FAILURE);
  } 

  if(fd_rdwr12 != -1) close(fd_rdwr12);
  if(fd_rdwr21 != -1) close(fd_rdwr21);

  return 0;
}
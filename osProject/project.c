
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#define CHILD_NUMBER 4

void create_project_team(int fd[6][2]);
void project_meeting_request(int fd[6][2]);
void print_meeting_schedule(int fd[6][2]);
void exit_PMS(int fd[6][2]);

void FCFS(int fd[6][2]);
void SJF(int fd[6][2]);
void analyse(int fd[6][2]);
void output(int fd[6][2]);




FILE *input_file;

int main() {
    int i;
    int fd[6][2];
    int pid = 0;
    int index = 0;
    
    
    for (i = 0; i < 6; i++) {
        if (pipe(fd[i]) < 0) {
            printf("Pipe creation error\n");
            exit(1);
        }
    }
    
    
    for (i = 1; i <= CHILD_NUMBER; i++) {
        pid = fork();
        if (pid == 0 || pid < 0) {
            if (pid == 0) {
                index = i;
                break;
            }
        }
    }
    
    if (pid < 0) {
        printf("Fork Failed\n");
        exit(1);
    }
    
    else if (pid == 0){
        if (index == 0) {
            create_project_team(fd);
            exit(0);
        }
        if (index == 1) {
            project_meeting_request(fd);
            exit(0);
        }
        if (index == 2) {
            print_meeting_schedule(fd);
            exit(0);
        }
        if (index == 3) {
            exit_PMS(fd);
            exit(0);
        }
    }
    if (pid > 0) {
        char buf[100];
        int option;
        printf("~~ WELCOME TO PolyStar ~~\n");
    }
    return 0;
}

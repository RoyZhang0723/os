#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#define CHILD_NUMBER 13


int extract_int(char *s, int start, int len);
int year(int yr);
int month(int yr, int mth);
int day(char *s1, char *s2);
char *IntToString(int num, char *str);
char *IntToDay(char *startDate, int start, int num);




void create_project_team(int fd[13][2], char* command);
void single_input_meeting_request(int fd[13][2], char* command);
void batch_input_meeting_request(int fd[13][2], char* command);
void meeting_attendance_request(int fd[13][2]);
void FCFS(int fd[13][2]);
void SJF(int fd[13][2]);
void analyse_attendance(int fd[13][2]);




FILE *input_file;


int main() {
    int i;
    int toChild[13][2];
    int toParent[13][2];
    int pid = 0;
    int index = 0;
    
    
    for (i = 0; i < 13; i++) {
        if (pipe(toChild[i]) < 0) {
            printf("Pipe creation error\n");
            exit(1);
        }
    }
    
    for (i = 0; i < 13; i++) {
        if (pipe(toParent[i]) < 0) {
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
            
            exit(0);
        }
        if (index == 1) {
            exit(0);
        }
        if (index == 2) {
            exit(0);
        }
        if (index == 3) {
            exit(0);
        }
        if (index == 4) {
            exit(0);
        }
        if (index == 5) {
            exit(0);
        }
        if (index == 6) {
            exit(0);
        }
        if (index == 7) {
            exit(0);
        }
        if (index == 8) {
            exit(0);
        }
        if (index == 9) {
            exit(0);
        }
        if (index == 10) {
            exit(0);
        }
        if (index == 11) {
            exit(0);
        }
        if (index == 12) {
            exit(0);
        }
        
    }
    if (pid > 0) {
        char buf[100];
        char option[2];
        char command[100];
        int len;
        for (i = 0; i < 13; i++) {
            close(toChild[i][0]);
            close(toParent[i][1]);
        }
        while (true) {
            printf("    ~~ WELCOME TO PolyStar ~~\n");
            printf("1.   Create Project Team\n");
            printf("2.   Project Meeting Request\n");
            printf("2a.  Single input\n2b.  Batch input\n2c.  Meeting Attendance\n\n");
            printf("3.  Print Meeting Schedule\n3a.  FCFS (First Come First Served)\n3b.  XXXX (Another algorithm implemented)\n3c.  YYYY (Attendance Report) \n\n");
            printf("4.  Exit\n\n");
            printf("Enter an option: ");
            len = read(0, command, 100);
            command[--len] = 0;
            strncpy(option, command, 1);
            if (strncmp(option, "1", 1) == 0) {
                while (true) {
                    len = read(0, command, 100);
                    command[--len] = 0;
                    if (strncmp(command, "0", 1)) {
                        break;
                    } else {
                        create_project_team(toChild, command);
                    }
                }

            }
            else if (strncmp(option, "2", 1) == 0) {
                while (true) {
                    len = read(0, command, 100);
                    if (strncmp(command, "For 2a", 6) == 0) {
                        single_input_meeting_request(toChild, command);
                    } else if (strncmp(command, "For 2b", 6) == 0) {
                        batch_input_meeting_request(toChild, command);
                    } else {
                        meeting_attendance_request(toChild);
                    }
                }
            }
            else if (strncmp(option, "3", 1) == 0) {
                while (true) {
                    len = read(0, command, 100);
                    if (strncmp(command, "For 3a", 6) == 0) {
                        FCFS(toChild);
                    } else if (strncmp(command, "For 3b", 6) == 0) {
                        SJF(toChild);
                    } else {
                        analyse_attendance(toChild);
                    }
                }
            }
            else if (strncmp(option, "4", 1) == 0) {
                break;
            }
            else if (strncmp(option, "0", 1) == 0){
                continue;
            }
        }
    }
    return 0;
}


int extract_int(char *s, int start, int len) {
    char ss[5];
    strncpy(ss, s + start, len);
    ss[len] = 0;
    return atoi(ss);
}


int year(int yr) {
    if ((yr % 400 == 0) || (yr % 100 != 0 && yr % 4 == 0))
        return 1;
    else
        return 0;
}


int month(int yr, int mth) {
    int days[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (mth != 2)
        return days[mth];
    else
        return days[mth] + year(yr);
}


int day(char *s1, char *s2) {

    int yr1, mth1, dy1;
    int yr2, mth2, dy2;
    int d_yr = 0, d_mth = 0, d_dy = 0;

    yr1 = extract_int(s1, 0, 4);
    mth1 = extract_int(s1, 5, 2);
    dy1 = extract_int(s1, 8, 2);

    yr2 = extract_int(s2, 0, 4);
    mth2 = extract_int(s2, 5, 2);
    dy2 = extract_int(s2, 8, 2);

    if ((yr1 > yr2) || (yr1 == yr2 && mth1 > mth2) || (yr1 == yr2 && mth1 == mth2 && dy1 > dy2))
        return -1;

    if (mth1 > 12 || mth2 > 12 || month(yr1, mth1) < dy1 || month(yr2, mth2) < dy2)
        return -1;

    int i;

    for (i = yr1 + 1; i <= yr2 - 1; i++) {
        d_yr = d_yr + 365 + year(i);
    }

    if (yr1 == yr2) {
        for (i = mth1; i < mth2; i++)
            d_mth = d_mth + month(yr1, i);
    }
    else {
        for (i = mth1; i <= 12; i++) {
            d_mth = d_mth + month(yr1, i);
        }
        for (i = 1; i < mth2; i++) {
            d_mth = d_mth + month(yr2, i);
        }
    }

    if (yr1 == yr2 && mth1 == mth2) {
        d_dy = dy2 - dy1 + 1;
    }
    else {
        d_dy = -dy1 + 1 + dy2;
    }

    return d_yr + d_mth + d_dy;

}


char *IntToString(int num, char *str) {
    int length = 0;
    do {
        str[length] = num % 10 + 48;
        length++;
        num /= 10;
    } while (num != 0);

    int j = 0;
    str[length] = '\0';
    for (; j < length / 2; j++) {
        char temp;
        temp = str[j];
        str[j] = str[length - 1 - j];
        str[length - 1 - j] = temp;
    }
    return str;
}


char *IntToDay(char *startDate, int start, int num) {
    char FIRST_DAY[11] = "xxxx-01-01";
    char year[4];
    int  YEAR, days = 365, i = 0, thisDate, j;
    int  month[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
    char mon[12][2] = { "01","02","03","04","05","06","07","08","09","10","11","12" };
    char day[31][2] = { "01","02","03","04","05","06","07","08","09","10","11","12","13","14","15","16","17","18","19","20","21","22","23","24","25","26","27","28","29","30","31" };
    for (j = 0; j < 4; j++) { FIRST_DAY[j] = startDate[j]; year[j] = startDate[j]; }
    YEAR = atoi(year);
    if (start == -1) printf("The date is illegal. Please reenter the comment.\n");
    else {
        thisDate = start + num;
        if (YEAR % 4 == 0) { days += 1; month[1] = 29; }
        if (start + num <= days) {
            i = 0;
            while (thisDate > month[i]) {
                thisDate -= month[i];
                i++;
            }
            for (j = 0; j < 2; j++) startDate[j + 5] = mon[i][j];
            for (j = 0; j < 2; j++) startDate[j + 8] = day[thisDate - 1][j];
        }
        else {
            printf("The date is illegal. Please reenter the comment.\n");
        }
    }
    return startDate;
}

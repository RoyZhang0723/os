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
#define MAX_LINE 162


struct Staff
{
    int Project[5];
    int Manger;
    int Team[5];
    int attend_team_number;
};

struct Project
{
    int Member[4];
    int Staff_number;
};

char create_pro_team[3] = {'c', 'p', 0};
char project_meeting_book[3] = {'p', 'b', 0};
char meeting_attend[3] = {'m', 'a', 0};
char fcfs[3] = {'f', 'f', 0};
char sjf[3] = {'s', 'j', 0};
char exit_PMS[3] = {'e', 'x', 0};




int extract_int(char *s, int start, int len);
int year(int yr);
int month(int yr, int mth);
bool is_vaild_day(char *s1);
bool is_vaild_time(char *s1);
int day(char *s1, char *s2);
char *IntToString(int num, char *str);
char *IntToDay(char *startDate, int start, int num);
char** split(char *str, char *delimiter);



void create_project_team(int fd[13][2], char* command, int len);
int single_input_meeting_request(int fd[13][2], char* useful_inf);
int batch_input_meeting_request(int fd[13][2], char* useful_inf);
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
    input_file = ("Input_Meeting.txt", "w");
    
    struct Staff staff[8];
    struct Project project[5];
    
    
    for (i = 0; i < 8; i++) {
        staff[i].attend_team_number = 0;
    }
    
    for (i = 0; i < 5; i++) {
        project[i].Staff_number = 0;
    }
    
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
        for (i = 0; i < CHILD_NUMBER; i++) {
            if (i == index) {
                close(toParent[i][0]);
                close(toChild[i][1]);
            } else {
                close(toParent[i][0]);
                close(toParent[i][1]);
                close(toChild[i][0]);
                close(toChild[i][1]);
            }
        }
        char buf[100];
        int num;
        
        char opertion[3];
        char information[8];
        while (true) {
            if ((num = read(toChild[index][0], buf, 10)) > 0) {
                opertion[0] = buf[0];
                opertion[1] = buf[1];
                opertion[2] = 0;
                strncpy(information, buf + 2, 6);
                
                if (index < 8) {
                    if (strcmp(opertion, create_pro_team) == 0) {
                        if (strlen(information) == 2) {
                            staff[index].Project[staff[index].attend_team_number] = information[1] - 'A';
                            staff[index].Team[staff[index].attend_team_number] = information[0] - 'A';
                            staff[index].attend_team_number++;
                        }
                        if (strlen(information) == 4) {
                            staff[index].Project[staff[index].attend_team_number] = information[1] - 'A';
                            staff[index].Team[staff[index].attend_team_number] = information[0] - 'A';
                            staff[index].attend_team_number++;
                            staff[index].Manger = information[0] - 'A';
                        }
                    }
                    if (strcmp(opertion, exit_PMS) == 0) {
                        break;
                    }
                } else {
                    if (strcmp(opertion, create_pro_team) == 0) {
                        int staff_number = strlen(information);
                        i = 0;
                        while (staff_number > 0) {
                            project[index].Member[project[index].Staff_number] = information[i] - 'A';
                            project[index].Staff_number++;
                            staff_number--;
                            i++;
                        }
                    }
                    if (strcmp(opertion, exit_PMS) == 0) {
                        break;
                    }
                }
                
            }
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
                        create_project_team(toChild, command, len);
                    }
                }

            }
            else if (strncmp(option, "2", 1) == 0) {
                while (true) {
                    len = read(0, command, 100);
                    if (strncmp(command, "For 2a", 6) == 0) {
                        char useful_inf[30];
                        strncpy(useful_inf, command + 8, 30);
                        if (single_input_meeting_request(toChild, useful_inf) < 0) {
                            printf("Invaild time\n");
                        } else {
                            printf("Record\n");
                        }
                    } else if (strncmp(command, "For 2b", 6) == 0) {
                        char useful_inf[30];
                        strncpy(useful_inf, command + 8, 30);
                        int record_num = 0;
                        record_num = batch_input_meeting_request(toChild, useful_inf);
                        printf("%d meeting requests have been recorded", record_num);
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
                for (i = 0; i < 13; i++) {
                    char temp[5];
                    strcpy(temp, exit_PMS);
                    write(toChild[i][1], temp, strlen(temp));
                }
                break;
            }
        }
    }
    return 0;
}

void create_project_team(int fd[13][2], char* command, int len) {
    char useful_inf[6];
    char** res = split(command, " ");
    int i = 0;
    while (res[i] != NULL) {
        if (i == 0) {
            useful_inf[i] = res[i][5];
        } else if (i == 1) {
            useful_inf[i] = res[i][8];
        } else {
            useful_inf[i] = res[i][0];
        }
        i++;
    }
    int useful_inf_len = i;
    char to_member_message[10];
    char to_manager_message[10];
    char to_project_message[10];
    char temp[5];
    char mananger_message[2] = "MM";
    strcpy(to_member_message, create_pro_team);
    strcpy(to_manager_message, create_pro_team);
    strcpy(to_project_message, create_pro_team);
    strncpy(temp, useful_inf, 2);
    strcat(to_member_message, temp);
    strcat(to_manager_message, temp);
    strcat(to_manager_message, mananger_message);
    strncpy(temp, useful_inf + 2, 4);
    strcat(to_project_message, temp);
    for (i = 2; i < useful_inf_len; i++) {
        if (i == 2) {
            write(fd[useful_inf[i] - 'A'], to_manager_message, strlen(to_manager_message));
        } else {
            write(fd[useful_inf[i] - 'A'], to_member_message, strlen(to_member_message));
        }
    }
    write(fd[useful_inf[1] - 'A' + 8], to_project_message, strlen(to_project_message));
}

int single_input_meeting_request(int fd[13][2], char* useful_inf) {
    char team[2], day[11], start_time[6], hours[2];
    strncpy(team, useful_inf + 5, 1);
    strncpy(day, useful_inf + 7, 10);
    strncpy(start_time, useful_inf + 18, 5);
    strncpy(hours, useful_inf + 24, 1);
    if (!is_vaild_day(day)) {
        return -1;
    }
    if (!is_vaild_time(start_time)) {
        return -1;
    }
    fprintf(input_file, "%s\n", useful_inf);
    return 1;
}


int batch_input_meeting_request(int fd[13][2], char* command) {
    char useful_inf[30];
    strncpy(useful_inf, command + 8, 30);
    int i = 0, line_num = 0, len = 0;
    char buf[MAX_LINE];
    char use_inf[30];
    FILE *fp;
    if ((fp = fopen(useful_inf, "r")) == NULL) {
        printf("fail to read the file");
        return -1;
    }
    
    while (fgets(buf, MAX_LINE, fp) != NULL) {
        len = strlen(buf);
        buf[len - 1] = '\0';
        strcpy(use_inf, buf);
        if (single_input_meeting_request(fd, use_inf) > 0) {
            line_num++;
        }
    }
    fclose(fp);
    return line_num;
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

bool is_vaild_day(char *s1) {
    int yr, mth, dy;
    int mth_and_day;
    yr = extract_int(s1, 0, 4);
    mth = extract_int(s1, 5, 2);
    dy = extract_int(s1, 8, 2);
    if (yr != 2022) {
        return false;
    }
    mth_and_day = mth * 100 + dy;
    if (mth < 415 || mth > 514) {
        return false;
    }
    return true;
}

bool is_vaild_time(char *s1) {
    int time;
    time =extract_int(s1, 0, 2);
    if (time < 9 || time > 18) {
        return false;
    }
    return true;
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

char** split(char *str, char *delimiter) {
    int len = strlen(str);
    char *strCopy = (char*)malloc((len + 1) * sizeof(char));
    strcpy(strCopy, str);
    for (int i = 0; strCopy[i] != '\0'; i++) {
        for (int j = 0; delimiter[j] != '\0'; j++) {
            if (strCopy[i] == delimiter[j]) {
                strCopy[i] = '\0';
                break;
            }
        }
    }
    char** res = (char**)malloc((len + 2) * sizeof(char*));
    len++;
    int resI = 0;
    for (int i = 0; i < len; i++) {
        res[resI++] = strCopy + i;
        while (strCopy[i] != '\0') {
            i++;
        }
    }
    res[resI] = NULL;
    return res;
}

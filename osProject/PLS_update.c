#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#define maxOrder 1000
#define MAX_LINE 100
#define CHILD_NUM 4

int extract_int(char *s, int start, int len);
int year(int yr);
int month(int yr, int mth);
int day(char *s1, char*s2);
char *IntToString(int num, char *str);
char *IntToDay(char *startDate, int start, int num);

void addPERIOD(char comment[100], char start_date[11], char end_date[11]);
int addORDER(char start_date[11], char end_date[11], char comment[100], char order[maxOrder][50], int order_num, int line_num);
int addBATCH(char start_date[11], char end_date[11], char comment[100], char order[maxOrder][50], int order_num);
void runPLS(char start_date[11], char end_date[11], char comment[100], char order[maxOrder][50], int order_num, int fdits[2][2], char type);
void write_InputToSchedule(char algorithm[10], char report_file[30], char start_date[11], char end_date[11], char order[maxOrder][50], int order_num, int fdits[2][2], char type);

void schedule_FDFSJF(int fdits[6][2]);
void schedule_FCFS(int fdits[6][2]);
void analyse(int fdits[6][2]);
void output(int fdits[6][2]);

FILE *inputfile;

// change the main1 to main if you want to run this code
int main1(void) {

    int i;
    int fdits[6][2];
    // pipe from InputModule to ScheduleModule (fdits[0] for algorithm 1, fdits[1] for algorithm 2, fdits[2] for output process, fdits[3] for analyse process
    // fdits[4] for telling the parent process the child process has finished.)
    int pid;
    inputfile = fopen("Input_Orders.txt", "w");

    //Create Pipe Channel
    for (i = 0; i < 6; i++) {
        if (pipe(fdits[i]) < 0) {
            printf("Pipe creation error\n");
            exit(1);
        }
    }

    //Create Child Process
    int childpid[4];
    for (i = 1; i <= CHILD_NUM; i++) {
        pid = fork();
        if (pid == 0 || pid < 0) {
            if (pid == 0) childpid[i] = getpid();
            break;
        }
    }

    // Fork Failed
    if (pid < 0) {
        printf("Fork Failed\n");
        exit(1);
    }

    // Child Process
    else if (pid == 0) {
        int mypid = getpid();
        for (i = 1; i <= CHILD_NUM; i++)
            if (childpid[i] == mypid) break;
        int no = i;
        
        // ScheduleModule: algorithm FCFS
        if (no == 1) {
            schedule_FCFS(fdits);
            //printf("FCFS out!!!\n");
            exit(0);
        }
        // ScheduleModule: algorithm FDFSJF (First Due First & Shortest Job First)
        if (no == 2) {
            schedule_FDFSJF(fdits);
            //printf("FDFSJF OUT!!!\n");
            exit(0);
        }
        // AnalyseModule
        if (no == 3) {
            analyse(fdits);
            //printf("Analyse OUT!!!\n");
            exit(0);
        }
        // OutputModule
        if (no == 4) {
            output(fdits);
            //printf("Output out!!!\n");
            exit(0);
        }
    }

    
    // Parent Process ( InputModule )
    if (pid > 0) {
        char utilization_c[5];
        float maxUtilization=0;
        char maxAlgorithm[10];
        char buffer[100];

        // close: InputModule in
        for (i = 0; i < 2; i++)
            close(fdits[i][0]);
        close(fdits[5][1]);

        char start_date[11], end_date[11]; // store: start date, end date
        char order[maxOrder][50]; // store: all orders
        int order_num=0; // store: number of order

        printf("~~WELCOME TO PLS~~\n");

        char comment[100]; // store: read-in comment
        int len;

        
        // Input Loop (exit until read the comment "exit")
        char type[5];
        while (1) {
            printf("Please enter:\n");
    
            // read: comments
            len = read(0, comment, 100);
            comment[--len] = 0;
            //printf("comment: %s \n", comment);
            // extract: type of comments ("addP", "addO", "addR", "exit")
            strncpy(type, comment, 4);
            // comment type: addPERIOD
            if (strncmp(type, "addP",4) == 0)
                addPERIOD(comment, start_date, end_date);
            

            // comment type: addPERIOD
            else if (strncmp(type, "addO",4) == 0)
                order_num = addORDER(start_date, end_date, comment, order, order_num, 0);

            
            // comment type: addBATCH
            else if (strncmp(type, "addB",4) == 0)
                order_num = addBATCH(start_date, end_date, comment, order, order_num);
    
            // comment type: runPLS
            else if (strncmp(type, "runP",4) == 0){
                
                runPLS(start_date, end_date, comment, order, order_num, fdits, 'a');
                int n=read(fdits[5][0], buffer, 100);
                buffer[n]=0;
                //printf("buffer is %s\n", buffer);
                if(maxUtilization==0){
                    if(buffer[4]>='A' && buffer[4] <= 'Z'){
                        strcpy(maxAlgorithm, "FDFSJF");
                        maxUtilization= strtof(buffer+6, NULL);
                       // printf("maxAlgorith %s maxultilization %f\n", maxAlgorithm, maxUtilization);
                    }
                    else{
                        strcpy(maxAlgorithm, "FCFS");
                        maxUtilization= strtof(buffer+4, NULL);
                        //printf("maxAlgorith %s ultilization %f\n", maxAlgorithm, maxUtilization);
                    }
                }
                else{
                    if(buffer[4]>='A' && buffer[4] <= 'Z'){
                        float utilization= strtof(buffer+6, NULL);
                        if(utilization > maxUtilization){
                            strcpy(maxAlgorithm, "FDFSJF");
                            maxUtilization=utilization;
                           // printf("maxAlgorith %s ultilization %f\n", maxAlgorithm, maxUtilization);
                        }
                    }
                    else{
                        float utilization= strtof(buffer+4, NULL);
                        if(utilization > maxUtilization){
                            strcpy(maxAlgorithm, "FCFS");
                            maxUtilization=utilization;
                            //printf("maxAlgorith %s ultilization %f\n", maxAlgorithm, maxUtilization);
                        }
                    }
                }
                    
                
                
                //strcpy(maxAlgorithm, buffer);
                //printf("\nMax algorithm %s\n", maxAlgorithm);
                
                while(1){
                    len = read(fdits[4][0], comment, 100);
                    if (len > 1) {
                        memset(comment, '\0', sizeof(comment));
                        break;}
                }
            }
            
            
            // comment type: exitPLS
            else if (strncmp(type, "exit",4) == 0) {
                if(strcmp(maxAlgorithm, "FCFS")==0){
                  write_InputToSchedule("FCFS","",start_date,end_date,order,order_num,fdits,'o');
                  write_InputToSchedule("FDFSJF","",start_date,end_date,order,order_num,fdits,'O');
                }
                else{
                  write_InputToSchedule("FDFSJF","",start_date,end_date,order,order_num,fdits,'o');
                  write_InputToSchedule("FCFS","",start_date,end_date,order,order_num,fdits,'O');

                }
                //write_InputToSchedule("FDFSJF","",start_date,end_date,order,order_num,fdits,'o');
                //runPLS(start_date, end_date, comment, order, order_num, fdits, 'o');
                close(fdits[5][0]);
                for (i = 1; i <= CHILD_NUM; i++)
                    wait(NULL);
                break;
            }
            
            // comment type: illegal
            else {
                printf("The comment is illegal. Please reenter the comment.\n");
            }

        }
        
        // close: InputModule out
        for (i = 0; i < 2; i++)
            close(fdits[i][1]);
        
    }
    
    fclose(inputfile);
    return 0;
}



/*
    comment type: addPERIOD
    extract start_date and end_date from comment
*/
void addPERIOD(char comment[100], char start_date[11], char end_date[11]){

    // extract: start_date, end_date
    strncpy(start_date, comment + 10, 10);
    start_date[10] = 0;
    strncpy(end_date, comment + 21, 10);
    end_date[10] = 0;
    //handle: illegal dates
    if (day(start_date, end_date) == -1){
        printf("The date is illegal. Please reenter the comment.\n");
    }
}

/*
    comment type: addORDER
    extract order from comment and add into the list of orders
    return the number of orders in the list
*/
int addORDER(char start_date[11], char end_date[11], char comment[100], char order[maxOrder][50], int order_num, int line_num) {
    char due_date[11];
    char task[50]; // store: present order

    // store all input in file
    fprintf(inputfile, "%s\n", comment);

    //for (j = 0; j < 4; j++) FIRST_DAY[j] = start_date[j];
    //start = day(FIRST_DAY, start_date);
    //fputs("\n***PLS Schedule Analysis Report***\n\n", fp);
    //fprintf(fp, "Algorithm used: %s\n\n", algorithm);

    // extract: due_date
    strncpy(due_date, comment + 15, 10);
    // handle: illegal dates (the due date is illegal or out of the boundary of start_date-end_date )
    if (day(start_date, due_date) == -1 || day(due_date, end_date) == -1) {
        if (line_num == 0) printf("The due date is out of boundary or illegal. Please reenter the comment.\n");
        else printf("The due date of comment in line %d is out of boundary or illegal.\n", line_num);
    }
    // extract: order information (format: order number + due date + quantity + product name)
    else {
        strncpy(task, comment + 9, 31);
        task[31] = '\0';
        // add present order to the list of orders
        strcpy(order[order_num++], task);
    }

    return order_num;
}

/*
    comment type: addBATCH
    extract list of comments from a file, then extract each order in the comment and add into the list of orders
    return the number of orders in the list
*/
int addBATCH(char start_date[11], char end_date[11], char comment[100], char order[maxOrder][50], int order_num) {
    // extract: file name of report
    char file_name[30];
    int i = 0;
    
    for (i = 9; i < 100; i++){
        if(comment[i] == '.'){
            break;
        }
    }
    i += 4;
    strncpy(file_name, comment + 9, i - 9);
    file_name[i - 9] = 0;
    printf("FILE NAME: %s\n", file_name);
    printf("COMMENT: %s\n", comment);
    char due_date[11];
    char task[50];
    int line_num = 0;
    int len;

    char buf[MAX_LINE];
    FILE *fp;
    
    if ((fp = fopen(file_name, "r")) == NULL)
    {
        perror("fail to read the file");
        exit(1);
    }

    while (fgets(buf, MAX_LINE, fp) != NULL)
    {
        line_num = line_num + 1;
        len = strlen(buf);
        buf[len - 1] = '\0';
        order_num = addORDER(start_date, end_date, buf, order, order_num, line_num);
    }
    fclose(fp);
    return order_num;
}

/*
    comment type: runPLS
    write information into the pipe from InputModule to ScheduleModule
*/
void runPLS(char start_date[11], char end_date[11], char comment[100], char order[maxOrder][50], int order_num, int fdits[2][2], char type) {
    char algorithm[10], report_file[30];

    int num = 0, j;

    // extract algorithm and file name of report from comment
    char *p;
    p = strtok(comment, " ");
    while (p)
    {
        num = num + 1;
        if (num == 2) {
            for (j = 0; j < strlen(p); j++) algorithm[j] = p[j];
            algorithm[j] = 0;
        }
        if (num == 6) {
            for (j = 0; j < strlen(p); j++) report_file[j] = p[j];
            report_file[j] = 0;
        }
        p = strtok(NULL, " ");
    }

    // write information to pipe from InputModule to ScheduleModule
    write_InputToSchedule(algorithm, report_file, start_date, end_date, order, order_num, fdits, type);
}

/*
    write information from InputModule to ScheduleModule
*/
void write_InputToSchedule(char algorithm[10], char report_file[30], char start_date[11], char end_date[11], char order[maxOrder][50], int order_num, int fdits[2][2], char type){
    //printf("start transfer\n");
    int i, j;
    if (strcmp(algorithm, "FCFS") == 0) i = 0;
    else if (strcmp(algorithm, "FDFSJF") == 0) i = 1;

    // write information to pipe from InputModule to ScheduleModule i ( i=0 means FCFS, i=1 means SJF )
    if (type == 'a') {
        write(fdits[i][1], "a;", 2); // start ("a" means "AnalyseModule")
        write(fdits[i][1], report_file, strlen(report_file)); // file name of report
    }
    if (type == 'o') {
        write(fdits[i][1], "o;", 2); // start ("o" means "OutputModule")
    }
    write(fdits[i][1], ";", 1);

    write(fdits[i][1], start_date, strlen(start_date)); // start date
    write(fdits[i][1], " ", 1);

    write(fdits[i][1], end_date, strlen(end_date)); // end date
    write(fdits[i][1], ";", 1);

    char num[5];
    sprintf(num, "%d", order_num);
    write(fdits[i][1], num, strlen(num)); // order_num
    write(fdits[i][1], ";", 1);
    
    for (j = 0; j < order_num; j++) {
        write(fdits[i][1], order[j], strlen(order[j])); // order ( order number + due date + quantity + product name )
        write(fdits[i][1], ";", 1);
        
    }
    write(fdits[i][1], "e;", 2); // end /* ERROR 1: i -> 0*/

}


void schedule_FCFS(int fdits[6][2]){

    close(fdits[0][1]);
    while (1){
        //printf("child process: FDFSJF\n");
        int end = 0;
        int endAnalyse=0;
        int num = 0;
        char buf[128];                    /* buf is used to store the string read in temporarily */
        char start_date[11];              /* period stores the start date in string format */
        char end_date[11];                /* period stores the end date in string format */
        int  period_Int;                  /* period stores the number of days */
        char order[maxOrder][50];      /* Order information in string format */
        char order_Num[maxOrder][6];      /* Order Numbers in string format*/
        char order_Due_Char[maxOrder][11];/* Order due date in string format */
        int  order_Due_Int[maxOrder];      /* Order due date in integer format */
        int  order_Quantity[maxOrder];      /* Order quantity in interger format */
        char order_Name[maxOrder][15];      /* The name of the order in string format */
        char FIRST_DAY[11] = "xxxx-01-01";/* First day of some year */
        char report_file[30];                 /* Store report to a file.    */
        int plant_X = 300, plant_Y = 400, plant_Z = 500; /* Capacity for three plants. */
        int  plant_X_Day = 1, plant_Y_Day = 1, plant_Z_Day = 1;
        char NumOfDaysInUse[3][5];
        char orderNumChar[6];
        /* counter is used to decide if we are read in starting date or orders.   */
        /* orderNum is an index used to indicate which order we are read in now.  */
        /* writeToFile are set to 1 once after the file name for output is known. */
        int counter = 0, orderNum = 0, writeToAnalyse = 0, writeToOutPut = 0, count = 0, rejectCount = 0, start;
        /* sortNow indicates all orders are read and the program is ready to sort.           */
        int sortNow = 0;
        /* Arrays used for output */
        int   plant_X_Work_SUM = 0, plant_Y_Work_SUM = 0, plant_Z_Work_SUM = 0;
        char  plant_Work_SUM_Char[3][10];
        int   plant_X_Work[maxOrder];
        int   plant_Y_Work[maxOrder];
        int   plant_Z_Work[maxOrder];
        char  plant_X_Work_Char[maxOrder][5];
        char  plant_Y_Work_Char[maxOrder][5];
        char  plant_Z_Work_Char[maxOrder][5];
        char  plant_X_OrderName[maxOrder][10];
        char  plant_Y_OrderName[maxOrder][10];
        char  plant_Z_OrderName[maxOrder][10];
    
        char  plant_X_OrderNum[maxOrder][6];
        char  plant_Y_OrderNum[maxOrder][6];
        char  plant_Z_OrderNum[maxOrder][6];

        char  plant_X_DueDate[maxOrder][11];
        char  plant_Y_DueDate[maxOrder][11];
        char  plant_Z_DueDate[maxOrder][11];
        /* Arrays used for analyse report */
        int  rejectedOrder[maxOrder];
        char order_Num_A[maxOrder][6];
        char order_Start_Day[maxOrder][5];
        char order_Start_Day_Char[maxOrder][11];
        char order_End_Day[maxOrder][5];
        char order_End_Day_Char[maxOrder][11];
        char order_Days[maxOrder][5];
        char order_Quantity_A[maxOrder][5];
        char order_Plant[maxOrder];
        int countFlag = 0, len;
        char newBuf[100] = {};
        count = 0;
        countFlag = 0;
        memset(buf, '\0', sizeof(buf));
        memset(newBuf, '\0', sizeof(newBuf));
        
        while (1){
            read(fdits[0][0], buf, 1); // Read from parent process
            if (strcmp(buf, ";") == 0){
            if (strcmp(newBuf, "e") == 0){
                sortNow = 1;
                break;
            }
            if (count == 0 ) { // Read in output mode
            if(strcmp(newBuf, "a") == 0){ writeToAnalyse = 1; writeToOutPut=1; }
            if(strcmp(newBuf, "o") == 0){ writeToAnalyse=1; writeToOutPut = 1; endAnalyse=1;}
            if(strcmp(newBuf, "O") == 0){ end = 1;}
            }
            else if (count == 1 && writeToAnalyse) strcpy(report_file, newBuf);
            else if (count == 2) { // Read in start date & end date
                strncpy(start_date, newBuf, 10);
                start_date[10] = 0;
                strncpy(end_date, newBuf + 11, 10);
                end_date[10] = 0;
                }
            else if (count == 3) {
                strcpy(orderNumChar, newBuf);
                orderNum = atoi(newBuf);
            }
            else{
                int index = count - 4;
                strncpy(order[index], newBuf,40); /* Read in orders */
                char quantity[6];
                int here = 0;
                int  blank[3][2] = {};
                int j;
                for ( j = 0; j < 39; j++){
                    if (order[index][j] == ' '){
                        if(order[index][j+1] != ' '){
                            blank[here][1] = j + 1;
                            here++;
                        }
                    }
                    else{
                        if(order[index][j+1] == ' ') blank[here][0] = j + 1;
                    }
                }
                strncpy(order_Num[index], order[index] + 0, blank[0][0]);
                strncpy(order_Due_Char[index], order[index] + blank[0][1], blank[1][0] - blank[0][1]);
                strncpy(quantity, order[index] + blank[1][1], blank[2][0] - blank[1][1]);
                strncpy(order_Name[index], order[index] + blank[2][1], 9);
                order_Due_Int[index] = day(start_date, order_Due_Char[index]);
                order_Quantity[index] = atoi(quantity);
                }
                memset(newBuf, '\0', sizeof(newBuf));
                countFlag = 0;
                count++;
                }
            else{
                newBuf[countFlag] = buf[0];
                countFlag++;
            }
            memset(buf, '\0', sizeof(buf));
        }
        memset(buf, '\0', sizeof(buf));
        memset(newBuf, '\0', sizeof(newBuf));
        
        if(end) break;
        
        if (sortNow){
            int period = day( start_date, end_date);
            int minimum = 0, minimum_Index = 0, workLoad = 0, day_Num = 0,temp, temp_Index, dueDAY;
            int all_work = period*(1200);
            int i, j;
            count = 0;
            rejectCount = 0;
            /* Scheduling Here */
            for (i = 0; i < orderNum; i++){
                int due = day(start_date, order_Due_Char[i]);
                int workLeftBeforeDue = 0;
                workLeftBeforeDue = due*(1200) - (plant_X_Day-1)*300 - (plant_Y_Day-1)*400 - (plant_Z_Day-1)*500;
                if (workLeftBeforeDue >= order_Quantity[i]){
                    workLoad = order_Quantity[i];
                    int plant_X_WORKDAY = 0, plant_Y_WORKDAY = 0, plant_Z_WORKDAY = 0;
                    int plant_X_Quantity = 0, plant_Y_Quantity = 0, plant_Z_Quantity = 0;
                    int plant_X_Previous_Day = plant_X_Day;
                    int plant_Y_Previous_Day = plant_Y_Day;
                    int plant_Z_Previous_Day = plant_Z_Day;
                    while (workLoad > 0){
                        // Put orders in plant Z
                        if (plant_Z_Day <= due && (workLoad > 500 || (workLoad > 0 && plant_X_Day > due && plant_Y_Day > due))  && plant_Z_Day <= period){
                            if (workLoad >= 500){
                                plant_Z_Work[plant_Z_Day-1] = 500;
                                IntToString(500 ,plant_Z_Work_Char[plant_Z_Day-1]);
                                workLoad -= 500;
                                plant_Z_Quantity += 500;
                                plant_Z_Work_SUM += 500;
                            }
                            else{
                                plant_Z_Work[plant_Z_Day-1] = workLoad;
                                plant_Z_Quantity += workLoad;
                                plant_Z_Work_SUM += workLoad;
                                IntToString(workLoad ,plant_Z_Work_Char[plant_Z_Day-1]);
                                workLoad = 0;
                            }
                            strcpy(plant_Z_OrderName[plant_Z_Day-1], order_Name[i]);
                            strcpy(plant_Z_OrderNum[plant_Z_Day-1], order_Num[i]);
                            strcpy(plant_Z_DueDate[plant_Z_Day-1], order_Due_Char[i]);
                            plant_Z_Day += 1;
                            plant_Z_WORKDAY += 1;
                            all_work -= 500;
                            }
                        // Put orders in plant Y
                        if (plant_Y_Day <= due && (workLoad > 400 || (workLoad > 0 && (plant_X_Day > due || plant_Z_Day > due)))  && plant_Y_Day <= period){
                            if (workLoad >= 400){
                                plant_Y_Work[plant_Y_Day-1] = 400;
                                IntToString(400 ,plant_Y_Work_Char[plant_Y_Day-1]);
                                plant_Y_Quantity += 400;
                                plant_Y_Work_SUM += 400;
                                workLoad -= 400;
                            }
                            else{
                                plant_Y_Work[plant_Y_Day-1] = workLoad;
                                IntToString(workLoad ,plant_Y_Work_Char[plant_Y_Day-1]);
                                plant_Y_Quantity += workLoad;
                                plant_Y_Work_SUM += workLoad;
                                workLoad = 0;
                            }
                            strcpy(plant_Y_OrderName[plant_Y_Day-1], order_Name[i]);
                            strcpy(plant_Y_OrderNum[plant_Y_Day-1], order_Num[i]);
                            strcpy(plant_Y_DueDate[plant_Y_Day-1], order_Due_Char[i]);
                            plant_Y_Day += 1;
                            plant_Y_WORKDAY += 1;
                            all_work -= 400;
                        }
                        // Put orders in plant X
                        if (plant_X_Day <= due && workLoad > 0 && plant_X_Day <= period) {
                            if (workLoad >= 300){
                                plant_X_Work[plant_X_Day-1] = 300;
                                IntToString(300 ,plant_X_Work_Char[plant_X_Day-1]);
                                plant_X_Quantity += 300;
                                plant_X_Work_SUM += 300;
                                workLoad -= 300;
                            }
                            else{
                                plant_X_Work[plant_X_Day] = workLoad;
                                IntToString(workLoad ,plant_X_Work_Char[plant_X_Day-1]);
                                plant_X_Quantity += workLoad;
                                plant_X_Work_SUM += workLoad;
                                workLoad = 0;
                            }
                            strcpy(plant_X_OrderName[plant_X_Day-1], order_Name[i]);
                            strcpy(plant_X_OrderNum[plant_X_Day-1], order_Num[i]);
                            strcpy(plant_X_DueDate[plant_X_Day-1], order_Due_Char[i]);
                            plant_X_Day += 1;
                            plant_X_WORKDAY += 1;
                            all_work -= 300;
                        }
                    }
                    if (plant_X_WORKDAY != 0){
                        char temp[5];
                        strcpy(order_Num_A[count],order_Num[i]);
                        IntToString(plant_X_Previous_Day, temp);
                        strcpy(order_Start_Day[count], temp);
                        IntToString(plant_X_Day - 1, temp);
                        strcpy(order_End_Day[count], temp);
                        IntToString(plant_X_WORKDAY, temp);
                        strcpy(order_Days[count], temp);
                        IntToString(plant_X_Quantity, temp);
                        strcpy(order_Quantity_A[count], temp);
                        order_Plant[count] =  'X';
                        count++;
                    }
                    if (plant_Y_WORKDAY != 0){
                        char temp[5];
                        strcpy(order_Num_A[count],order_Num[i]);
                        
                        IntToString(plant_Y_Previous_Day, temp);
                        strcpy(order_Start_Day[count], temp);
                        IntToString(plant_Y_Day - 1, temp);
                        strcpy(order_End_Day[count], temp);
                        IntToString(plant_Y_WORKDAY, temp);
                        strcpy(order_Days[count], temp);
                        IntToString(plant_Y_Quantity, temp);
                        strcpy(order_Quantity_A[count], temp);
                        order_Plant[count] =  'Y';
                        count++;
                    }
                    if (plant_Z_WORKDAY != 0){
                        char temp[5];
                        strcpy(order_Num_A[count],order_Num[i]);
                        IntToString(plant_Z_Previous_Day, temp);
                        strcpy(order_Start_Day[count], temp);
                        IntToString(plant_Z_Day - 1, temp);
                        strcpy(order_End_Day[count], temp);
                        IntToString(plant_Z_WORKDAY, temp);
                        strcpy(order_Days[count], temp);
                        IntToString(plant_Z_Quantity, temp);
                        strcpy(order_Quantity_A[count], temp);
                        order_Plant[count] =  'Z';
                        count++;
                    }
                }
                else {
                    rejectedOrder[rejectCount] = i;
                    rejectCount++;
                }
                IntToString(plant_X_Day-1,NumOfDaysInUse[0]);
                IntToString(plant_Y_Day-1,NumOfDaysInUse[1]);
                IntToString(plant_Z_Day-1,NumOfDaysInUse[2]);
                IntToString(plant_X_Work_SUM,plant_Work_SUM_Char[0]);
                IntToString(plant_Y_Work_SUM,plant_Work_SUM_Char[1]);
                IntToString(plant_Z_Work_SUM,plant_Work_SUM_Char[2]);
                //printf("plant_Work_SUM_Char[0]:%s\n", plant_Work_SUM_Char[0]);
                //printf("plant_Work_SUM_Char[1]:%s\n", plant_Work_SUM_Char[1]);
                //printf("plant_Work_SUM_Char[2]:%s\n", plant_Work_SUM_Char[2]);
                for (j = 0; j < 4; j++) FIRST_DAY[j] = start_date[j];
                start = day(FIRST_DAY, start_date);
                for (j = 0; j < count; j++) {
                    char date[11];
                    strcpy(date, start_date);
                    IntToDay(date, start, atoi(order_Start_Day[j]) - 1);
                    strcpy(order_Start_Day_Char[j],date);
                }

                for (j = 0; j < count; j++) {
                    char date[11];
                    strcpy(date, start_date);
                    IntToDay(date, start, atoi(order_End_Day[j]) - 1);
                    strcpy(order_End_Day_Char[j],date);
                }
            }
        }
        // WRITE TO OUTPUT process.
        if (writeToOutPut){
            int j;
            //  Write out algorithm type
            write(fdits[2][1], "FCFS", 5); // start;
            write(fdits[2][1], ";", 1);
            //  Write out  start date and end date
            write(fdits[2][1], start_date, strlen(start_date));
            write(fdits[2][1], " ", 1);
            write(fdits[2][1], end_date, strlen(end_date));
            write(fdits[2][1], ";", 1);
            // Write out the number of days in use for X,Y,Z
            for (j = 0; j < 3; j++) {
                write(fdits[2][1], NumOfDaysInUse[j], strlen(NumOfDaysInUse[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out order names of orders which plant X finished
            for (j = 0; j < plant_X_Day-1; j++) {
                write(fdits[2][1], plant_X_OrderName[j], strlen(plant_X_OrderName[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out order number of orders which plant X finished
            for (j = 0; j < plant_X_Day-1; j++) {
                write(fdits[2][1], plant_X_OrderNum[j], strlen(plant_X_OrderNum[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out quantity of orders which plant X worked
            for (j = 0; j < plant_X_Day-1; j++) {
                write(fdits[2][1], plant_X_Work_Char[j], strlen(plant_X_Work_Char[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out due day of orders which plant X finished
            for (j = 0; j < plant_X_Day-1; j++) {
                write(fdits[2][1], plant_X_DueDate[j], strlen(plant_X_DueDate[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out order names of orders which plant Y finished
            for (j = 0; j < plant_Y_Day-1; j++) {
                write(fdits[2][1], plant_Y_OrderName[j], strlen(plant_Y_OrderName[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out order number of orders which plant Y finished
            for (j = 0; j < plant_Y_Day-1; j++) {
                write(fdits[2][1], plant_Y_OrderNum[j], strlen(plant_Y_OrderNum[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out quantity of orders which plant Y worked
            for (j = 0; j < plant_Y_Day-1; j++) {
                write(fdits[2][1], plant_Y_Work_Char[j], strlen(plant_Y_Work_Char[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out due day of orders which plant Y finished
            for (j = 0; j < plant_Y_Day-1; j++) {
                write(fdits[2][1], plant_Y_DueDate[j], strlen(plant_Y_DueDate[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out order names of orders which plant Z finished
            for (j = 0; j < plant_Z_Day-1; j++) {
                write(fdits[2][1], plant_Z_OrderName[j], strlen(plant_Z_OrderName[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out order number of orders which plant Z finished
            for (j = 0; j < plant_Z_Day-1; j++) {
                write(fdits[2][1], plant_Z_OrderNum[j], strlen(plant_Z_OrderNum[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out quantity of orders which plant Z worked
            for (j = 0; j < plant_Z_Day-1; j++) {
                write(fdits[2][1], plant_Z_Work_Char[j], strlen(plant_Z_Work_Char[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out due day of orders which plant Z finished
            for (j = 0; j < plant_Z_Day-1; j++) {
                write(fdits[2][1], plant_Z_DueDate[j], strlen(plant_Z_DueDate[j]));
                write(fdits[2][1], ";", 1);
            }
            if(endAnalyse!=1){
              write(fdits[2][1], "e;", 2);
            }
            else{
            write(fdits[2][1], "E;", 2); //ask output to stop
            //write(fdits[3][1], "o;", 2);
            //  break;
            }
        }
        // WRITE TO ANALYSE process.
        if (writeToAnalyse){
            int j;
            char ROW[6], rejectROW[6];
            IntToString(count, ROW);
            IntToString(rejectCount, rejectROW);
            // Write out algorithm type
            write(fdits[3][1], "FCFS;", 5);
            // Write out file name
            write(fdits[3][1], report_file, strlen(report_file));
            write(fdits[3][1], ";", 1);
            // Write out start date and end date
            write(fdits[3][1], start_date, strlen(start_date));
            write(fdits[3][1], " ", 1);
            write(fdits[3][1], end_date, strlen(end_date));
            write(fdits[3][1], ";", 1);
            // Write out the total number of orders
            write(fdits[3][1], orderNumChar, strlen(orderNumChar));
            write(fdits[3][1], ";", 1);
            // Write out the total number of rows for printing analyse report
            write(fdits[3][1], ROW, strlen(ROW));
            write(fdits[3][1], ";", 1);
            // Write out the order number
            for (j = 0; j < count; j++) {
                write(fdits[3][1], order_Num_A[j], strlen(order_Num_A[j]));
                write(fdits[3][1], ";", 1);
            }
            // Write out the order start day
            for (j = 0; j < count; j++) {
                write(fdits[3][1], order_Start_Day_Char[j], strlen(order_Start_Day_Char[j]));
                write(fdits[3][1], ";", 1);
            }
            //  Write out the order end day
            for (j = 0; j < count; j++) {
                write(fdits[3][1], order_End_Day_Char[j], strlen(order_End_Day_Char[j]));
                write(fdits[3][1], ";", 1);
            }
            //  Write out the order Quantity
            for (j = 0; j < count; j++) {
                write(fdits[3][1], order_Quantity_A[j], strlen(order_Quantity_A[j]));
                write(fdits[3][1], ";", 1);
            }
            // Write out the plant which produced the orders
            write(fdits[3][1], order_Plant, strlen(order_Plant));
            write(fdits[3][1], ";", 1);
            //  Write out the number of rejected orders
            write(fdits[3][1], rejectROW, strlen(rejectROW));
            write(fdits[3][1], ";", 1);
            // Write out the rejected order number
            for (j = 0; j < rejectCount; j++) {
                write(fdits[3][1], order_Num[rejectedOrder[j]], strlen(order_Num[rejectedOrder[j]]));
                write(fdits[3][1], ";", 1);
            }
            //  Write out the rejected order name
            for  (j = 0; j < rejectCount; j++) {
                write(fdits[3][1], order_Name[rejectedOrder[j]], strlen(order_Name[rejectedOrder[j]]));
                write(fdits[3][1], ";", 1);
            }
            //  Write out the rejected order due
            for (j = 0; j < rejectCount; j++) {
                write(fdits[3][1], order_Due_Char[rejectedOrder[j]], strlen(order_Due_Char[rejectedOrder[j]]));
                write(fdits[3][1], ";", 1);
            }
            // Write out the number of days in use for X,Y,Z
            for (j = 0; j < 3; j++) {
                write(fdits[3][1], NumOfDaysInUse[j], strlen(NumOfDaysInUse[j]));
                write(fdits[3][1], ";", 1);
            }
            // Write out the total work of X,Y and Z
            for (j = 0; j < 3; j++) {
                write(fdits[3][1], plant_Work_SUM_Char[j], strlen(plant_Work_SUM_Char[j]));
                write(fdits[3][1], ";", 1);
            }
            if(endAnalyse!=1){
              write(fdits[3][1], "e;", 2);
            }
            else{
            write(fdits[3][1], "E;", 2); //ask analyse to stop
            //write(fdits[3][1], "o;", 2);
            break;
            }
        }
        
    }
}


void schedule_FDFSJF(int fdits[6][2]){

    close(fdits[1][1]);
    while (1){
        //printf("child process: FDFSJF\n");
        int end = 0;
        int endAnalyse=0;
        int num = 0;
        char buf[128];                    /* buf is used to store the string read in temporarily */
        char start_date[11];              /* period stores the start date in string format */
        char end_date[11];                /* period stores the end date in string format */
        int  period_Int;                  /* period stores the number of days */
        char order[maxOrder][50];      /* Order information in string format */
        char order_Num[maxOrder][6];      /* Order Numbers in string format*/
        char order_Due_Char[maxOrder][11];/* Order due date in string format */
        int  order_Due_Int[maxOrder];      /* Order due date in integer format */
        int  order_Quantity[maxOrder];      /* Order quantity in interger format */
        char order_Name[maxOrder][15];      /* The name of the order in string format */
        int  order_Index[maxOrder];        /* Store the index after sorting */
        char FIRST_DAY[11] = "xxxx-01-01";/* First day of some year */
        char report_file[30];                 /* Store report to a file.    */
        int plant_X = 300, plant_Y = 400, plant_Z = 500; /* Capacity for three plants. */
        int  plant_X_Day = 1, plant_Y_Day = 1, plant_Z_Day = 1;
        char NumOfDaysInUse[3][5];
        char orderNumChar[6];
        /* counter is used to decide if we are read in starting date or orders.   */
        /* orderNum is an index used to indicate which order we are read in now.  */
        /* writeToFile are set to 1 once after the file name for output is known. */
        int counter = 0, orderNum = 0, writeToAnalyse = 0, writeToOutPut = 0, count = 0, rejectCount = 0, start;
        /* sortNow indicates all orders are read and the program is ready to sort.           */
        int sortNow = 0;
        /* Arrays used for output */
        int   plant_X_Work_SUM = 0, plant_Y_Work_SUM = 0, plant_Z_Work_SUM = 0;
        char  plant_Work_SUM_Char[3][10];
        int   plant_X_Work[maxOrder];
        int   plant_Y_Work[maxOrder];
        int   plant_Z_Work[maxOrder];
        char  plant_X_Work_Char[maxOrder][5];
        char  plant_Y_Work_Char[maxOrder][5];
        char  plant_Z_Work_Char[maxOrder][5];
        char  plant_X_OrderName[maxOrder][10];
        char  plant_Y_OrderName[maxOrder][10];
        char  plant_Z_OrderName[maxOrder][10];
    
        char  plant_X_OrderNum[maxOrder][6];
        char  plant_Y_OrderNum[maxOrder][6];
        char  plant_Z_OrderNum[maxOrder][6];

        char  plant_X_DueDate[maxOrder][11];
        char  plant_Y_DueDate[maxOrder][11];
        char  plant_Z_DueDate[maxOrder][11];
        /* Arrays used for analyse report */
        int  rejectedOrder[maxOrder];
        char order_Num_A[maxOrder][6];
        char order_Start_Day[maxOrder][5];
        char order_Start_Day_Char[maxOrder][11];
        char order_End_Day[maxOrder][5];
        char order_End_Day_Char[maxOrder][11];
        char order_Days[maxOrder][5];
        char order_Quantity_A[maxOrder][5];
        char order_Plant[maxOrder];
        int countFlag = 0, len;
        char newBuf[100] = {};
        count = 0;
        countFlag = 0;
        memset(buf, '\0', sizeof(buf));
        memset(newBuf, '\0', sizeof(newBuf));
        
        while (1){
            read(fdits[1][0], buf, 1); // Read from parent process
            if (strcmp(buf, ";") == 0){
            if (strcmp(newBuf, "e") == 0){
                sortNow = 1;
                break;
            }
            if (count == 0 ) { // Read in output mode
            if(strcmp(newBuf, "a") == 0){ writeToAnalyse = 1; writeToOutPut=1; }
            if(strcmp(newBuf, "o") == 0){ writeToAnalyse = 1; writeToOutPut = 1; endAnalyse=1;}
            if(strcmp(newBuf, "O") == 0){ end = 1;}
            }
            else if (count == 1 && writeToAnalyse) strcpy(report_file, newBuf);
            else if (count == 2) { // Read in start date & end date
                strncpy(start_date, newBuf, 10);
                start_date[10] = 0;
                strncpy(end_date, newBuf + 11, 10);
                end_date[10] = 0;
                }
            else if (count == 3) {
                strcpy(orderNumChar, newBuf);
                orderNum = atoi(newBuf);
            }
            else{
                int index = count - 4;
                strncpy(order[index], newBuf,40); /* Read in orders */
                char quantity[6];
                int here = 0;
                int  blank[3][2] = {};
                int j;
                for ( j = 0; j < 39; j++){
                    if (order[index][j] == ' '){
                        if(order[index][j+1] != ' '){
                            blank[here][1] = j + 1;
                            here++;
                        }
                    }
                    else{
                        if(order[index][j+1] == ' ') blank[here][0] = j + 1;
                    }
                }
                strncpy(order_Num[index], order[index] + 0, blank[0][0]);
                strncpy(order_Due_Char[index], order[index] + blank[0][1], blank[1][0] - blank[0][1]);
                strncpy(quantity, order[index] + blank[1][1], blank[2][0] - blank[1][1]);
                strncpy(order_Name[index], order[index] + blank[2][1], 9);
                order_Due_Int[index] = day(start_date, order_Due_Char[index]);
                order_Quantity[index] = atoi(quantity);
                }
                memset(newBuf, '\0', sizeof(newBuf));
                countFlag = 0;
                count++;
                }
            else{
                newBuf[countFlag] = buf[0];
                countFlag++;
            }
            memset(buf, '\0', sizeof(buf));
        }
        memset(buf, '\0', sizeof(buf));
        memset(newBuf, '\0', sizeof(newBuf));
        
        if(end) break;
        // Sort the orders
        if (sortNow){
            int period = day( start_date, end_date);
            int minimum = 0, minimum_Index = 0, workLoad = 0, day_Num = 0,temp, temp_Index, dueDAY;
            int all_work = period*(1200);
            int i, j;
            for (i = 0; i < orderNum; i++){
                order_Index[i] = i;
            }
            /* Sort the order quantities by using First Due First algorithm*/
            for (i = 0; i < orderNum - 1; i++){
                minimum = order_Due_Int[i];
                minimum_Index = i;
                for (j = i + 1; j < orderNum; j++){
                    if (order_Due_Int[j]  < minimum){
                        minimum_Index = j;
                        minimum = order_Due_Int[j];
                    }
                }
                temp = order_Due_Int[i];
                order_Due_Int[i] = order_Due_Int[minimum_Index];
                order_Due_Int[minimum_Index] = temp;
                temp = order_Quantity[i];
                order_Quantity[i] = order_Quantity[minimum_Index];
                order_Quantity[minimum_Index] = temp;
                temp_Index = order_Index[i];
                order_Index[i] = order_Index[minimum_Index];
                order_Index[minimum_Index] = temp_Index;
            }
            /* Sort the order quantities if more than two orders have same due date, then sort the orders bu using SJF algorithm*/
            for (i = 0; i < orderNum - 1; i++){
                dueDAY = order_Due_Int[i];
                minimum = order_Quantity[i];
                minimum_Index = i;
                for (j = i + 1; j < orderNum; j++){
                    if(dueDAY == order_Due_Int[j] && order_Quantity[j]  < minimum){
                        minimum_Index = j;
                        minimum = order_Quantity[j];
                    }
                        
                }
                temp = order_Quantity[i];
                order_Quantity[i] = order_Quantity[minimum_Index];
                order_Quantity[minimum_Index] = temp;
                temp_Index = order_Index[i];
                order_Index[i] = order_Index[minimum_Index];
                order_Index[minimum_Index] = temp_Index;
            }

            count = 0;
            rejectCount = 0;
            /* Scheduling Here */
            for (i = 0; i < orderNum; i++){
                int due = day(start_date, order_Due_Char[order_Index[i]]);
                int workLeftBeforeDue = 0;
                workLeftBeforeDue = due*(1200) - (plant_X_Day-1)*300 - (plant_Y_Day-1)*400 - (plant_Z_Day-1)*500;
                if (workLeftBeforeDue >= order_Quantity[i]){
                    workLoad = order_Quantity[i];
                    int plant_X_WORKDAY = 0, plant_Y_WORKDAY = 0, plant_Z_WORKDAY = 0;
                    int plant_X_Quantity = 0, plant_Y_Quantity = 0, plant_Z_Quantity = 0;
                    int plant_X_Previous_Day = plant_X_Day;
                    int plant_Y_Previous_Day = plant_Y_Day;
                    int plant_Z_Previous_Day = plant_Z_Day;
                    while (workLoad > 0){
                        // Put orders in plant Z
                        if (plant_Z_Day <= due && (workLoad > 500 || (workLoad > 0 && plant_X_Day > due && plant_Y_Day > due))  && plant_Z_Day <= period){
                            if (workLoad >= 500){
                                plant_Z_Work[plant_Z_Day-1] = 500;
                                IntToString(500 ,plant_Z_Work_Char[plant_Z_Day-1]);
                                workLoad -= 500;
                                plant_Z_Quantity += 500;
                                plant_Z_Work_SUM += 500;
                            }
                            else{
                                plant_Z_Work[plant_Z_Day-1] = workLoad;
                                plant_Z_Quantity += workLoad;
                                plant_Z_Work_SUM += workLoad;
                                IntToString(workLoad ,plant_Z_Work_Char[plant_Z_Day-1]);
                                workLoad = 0;
                            }
                            strcpy(plant_Z_OrderName[plant_Z_Day-1], order_Name[order_Index[i]]);
                            strcpy(plant_Z_OrderNum[plant_Z_Day-1], order_Num[order_Index[i]]);
                            strcpy(plant_Z_DueDate[plant_Z_Day-1], order_Due_Char[order_Index[i]]);
                            plant_Z_Day += 1;
                            plant_Z_WORKDAY += 1;
                            all_work -= 500;
                            }
                        // Put orders in plant Y
                        if (plant_Y_Day <= due && (workLoad > 400 || (workLoad > 0 && (plant_X_Day > due || plant_Z_Day > due)))  && plant_Y_Day <= period){
                            if (workLoad >= 400){
                                plant_Y_Work[plant_Y_Day-1] = 400;
                                IntToString(400 ,plant_Y_Work_Char[plant_Y_Day-1]);
                                plant_Y_Quantity += 400;
                                plant_Y_Work_SUM += 400;
                                workLoad -= 400;
                            }
                            else{
                                plant_Y_Work[plant_Y_Day-1] = workLoad;
                                IntToString(workLoad ,plant_Y_Work_Char[plant_Y_Day-1]);
                                plant_Y_Quantity += workLoad;
                                plant_Y_Work_SUM += workLoad;
                                workLoad = 0;
                            }
                            strcpy(plant_Y_OrderName[plant_Y_Day-1], order_Name[order_Index[i]]);
                            strcpy(plant_Y_OrderNum[plant_Y_Day-1], order_Num[order_Index[i]]);
                            strcpy(plant_Y_DueDate[plant_Y_Day-1], order_Due_Char[order_Index[i]]);
                            plant_Y_Day += 1;
                            plant_Y_WORKDAY += 1;
                            all_work -= 400;
                        }
                        // Put orders in plant X
                        if (plant_X_Day <= due && workLoad > 0 && plant_X_Day <= period) {
                            if (workLoad >= 300){
                                plant_X_Work[plant_X_Day-1] = 300;
                                IntToString(300 ,plant_X_Work_Char[plant_X_Day-1]);
                                plant_X_Quantity += 300;
                                plant_X_Work_SUM += 300;
                                workLoad -= 300;
                            }
                            else{
                                plant_X_Work[plant_X_Day] = workLoad;
                                IntToString(workLoad ,plant_X_Work_Char[plant_X_Day-1]);
                                plant_X_Quantity += workLoad;
                                plant_X_Work_SUM += workLoad;
                                workLoad = 0;
                            }
                            strcpy(plant_X_OrderName[plant_X_Day-1], order_Name[order_Index[i]]);
                            strcpy(plant_X_OrderNum[plant_X_Day-1], order_Num[order_Index[i]]);
                            strcpy(plant_X_DueDate[plant_X_Day-1], order_Due_Char[order_Index[i]]);
                            plant_X_Day += 1;
                            plant_X_WORKDAY += 1;
                            all_work -= 300;
                        }
                    }
                    if (plant_X_WORKDAY != 0){
                        char temp[5];
                        strcpy(order_Num_A[count],order_Num[order_Index[i]]);
                        IntToString(plant_X_Previous_Day, temp);
                        strcpy(order_Start_Day[count], temp);
                        IntToString(plant_X_Day - 1, temp);
                        strcpy(order_End_Day[count], temp);
                        IntToString(plant_X_WORKDAY, temp);
                        strcpy(order_Days[count], temp);
                        IntToString(plant_X_Quantity, temp);
                        strcpy(order_Quantity_A[count], temp);
                        order_Plant[count] =  'X';
                        count++;
                    }
                    if (plant_Y_WORKDAY != 0){
                        char temp[5];
                        strcpy(order_Num_A[count],order_Num[order_Index[i]]);
                        
                        IntToString(plant_Y_Previous_Day, temp);
                        strcpy(order_Start_Day[count], temp);
                        IntToString(plant_Y_Day - 1, temp);
                        strcpy(order_End_Day[count], temp);
                        IntToString(plant_Y_WORKDAY, temp);
                        strcpy(order_Days[count], temp);
                        IntToString(plant_Y_Quantity, temp);
                        strcpy(order_Quantity_A[count], temp);
                        order_Plant[count] =  'Y';
                        count++;
                    }
                    if (plant_Z_WORKDAY != 0){
                        char temp[5];
                        strcpy(order_Num_A[count],order_Num[order_Index[i]]);
                        IntToString(plant_Z_Previous_Day, temp);
                        strcpy(order_Start_Day[count], temp);
                        IntToString(plant_Z_Day - 1, temp);
                        strcpy(order_End_Day[count], temp);
                        IntToString(plant_Z_WORKDAY, temp);
                        strcpy(order_Days[count], temp);
                        IntToString(plant_Z_Quantity, temp);
                        strcpy(order_Quantity_A[count], temp);
                        order_Plant[count] =  'Z';
                        count++;
                    }
                }
                else {
                    rejectedOrder[rejectCount] = order_Index[i];
                    rejectCount++;
                }
                IntToString(plant_X_Day-1,NumOfDaysInUse[0]);
                IntToString(plant_Y_Day-1,NumOfDaysInUse[1]);
                IntToString(plant_Z_Day-1,NumOfDaysInUse[2]);
                IntToString(plant_X_Work_SUM,plant_Work_SUM_Char[0]);
                IntToString(plant_Y_Work_SUM,plant_Work_SUM_Char[1]);
                IntToString(plant_Z_Work_SUM,plant_Work_SUM_Char[2]);
                //printf("plant_Work_SUM_Char[0]:%s\n", plant_Work_SUM_Char[0]);
                //printf("plant_Work_SUM_Char[1]:%s\n", plant_Work_SUM_Char[1]);
                //printf("plant_Work_SUM_Char[2]:%s\n", plant_Work_SUM_Char[2]);
                for (j = 0; j < 4; j++) FIRST_DAY[j] = start_date[j];
                start = day(FIRST_DAY, start_date);
                for (j = 0; j < count; j++) {
                    char date[11];
                    strcpy(date, start_date);
                    IntToDay(date, start, atoi(order_Start_Day[j]) - 1);
                    strcpy(order_Start_Day_Char[j],date);
                }

                for (j = 0; j < count; j++) {
                    char date[11];
                    strcpy(date, start_date);
                    IntToDay(date, start, atoi(order_End_Day[j]) - 1);
                    strcpy(order_End_Day_Char[j],date);
                }
            }
        }
        // WRITE TO OUTPUT process.
        if (writeToOutPut){
            int j;
            //  Write out algorithm type
            write(fdits[2][1], "FDFSJF", 7); // start;
            write(fdits[2][1], ";", 1);
            //  Write out  start date and end date
            write(fdits[2][1], start_date, strlen(start_date));
            write(fdits[2][1], " ", 1);
            write(fdits[2][1], end_date, strlen(end_date));
            write(fdits[2][1], ";", 1);
            // Write out the number of days in use for X,Y,Z
            for (j = 0; j < 3; j++) {
                write(fdits[2][1], NumOfDaysInUse[j], strlen(NumOfDaysInUse[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out order names of orders which plant X finished
            for (j = 0; j < plant_X_Day-1; j++) {
                write(fdits[2][1], plant_X_OrderName[j], strlen(plant_X_OrderName[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out order number of orders which plant X finished
            for (j = 0; j < plant_X_Day-1; j++) {
                write(fdits[2][1], plant_X_OrderNum[j], strlen(plant_X_OrderNum[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out quantity of orders which plant X worked
            for (j = 0; j < plant_X_Day-1; j++) {
                write(fdits[2][1], plant_X_Work_Char[j], strlen(plant_X_Work_Char[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out due day of orders which plant X finished
            for (j = 0; j < plant_X_Day-1; j++) {
                write(fdits[2][1], plant_X_DueDate[j], strlen(plant_X_DueDate[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out order names of orders which plant Y finished
            for (j = 0; j < plant_Y_Day-1; j++) {
                write(fdits[2][1], plant_Y_OrderName[j], strlen(plant_Y_OrderName[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out order number of orders which plant Y finished
            for (j = 0; j < plant_Y_Day-1; j++) {
                write(fdits[2][1], plant_Y_OrderNum[j], strlen(plant_Y_OrderNum[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out quantity of orders which plant Y worked
            for (j = 0; j < plant_Y_Day-1; j++) {
                write(fdits[2][1], plant_Y_Work_Char[j], strlen(plant_Y_Work_Char[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out due day of orders which plant Y finished
            for (j = 0; j < plant_Y_Day-1; j++) {
                write(fdits[2][1], plant_Y_DueDate[j], strlen(plant_Y_DueDate[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out order names of orders which plant Z finished
            for (j = 0; j < plant_Z_Day-1; j++) {
                write(fdits[2][1], plant_Z_OrderName[j], strlen(plant_Z_OrderName[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out order number of orders which plant Z finished
            for (j = 0; j < plant_Z_Day-1; j++) {
                write(fdits[2][1], plant_Z_OrderNum[j], strlen(plant_Z_OrderNum[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out quantity of orders which plant Z worked
            for (j = 0; j < plant_Z_Day-1; j++) {
                write(fdits[2][1], plant_Z_Work_Char[j], strlen(plant_Z_Work_Char[j]));
                write(fdits[2][1], ";", 1);
            }
            // Write out due day of orders which plant Z finished
            for (j = 0; j < plant_Z_Day-1; j++) {
                write(fdits[2][1], plant_Z_DueDate[j], strlen(plant_Z_DueDate[j]));
                write(fdits[2][1], ";", 1);
            }
            if(endAnalyse!=1){
              write(fdits[2][1], "e;", 2);
            }
            else{
                write(fdits[2][1], "E;", 2); //let output terminate
                //write(fdits[3][1], "o;", 2);
                //break;
            }
        }
        // WRITE TO ANALYSE process.
        if (writeToAnalyse){
            int j;
            char ROW[6], rejectROW[6];
            IntToString(count, ROW);
            IntToString(rejectCount, rejectROW);
            // Write out algorithm type
            write(fdits[3][1], "FDFSJF;", 7);
            // Write out file name
            write(fdits[3][1], report_file, strlen(report_file));
            write(fdits[3][1], ";", 1);
            // Write out start date and end date
            write(fdits[3][1], start_date, strlen(start_date));
            write(fdits[3][1], " ", 1);
            write(fdits[3][1], end_date, strlen(end_date));
            write(fdits[3][1], ";", 1);
            // Write out the total number of orders
            write(fdits[3][1], orderNumChar, strlen(orderNumChar));
            write(fdits[3][1], ";", 1);
            // Write out the total number of rows for printing analyse report
            write(fdits[3][1], ROW, strlen(ROW));
            write(fdits[3][1], ";", 1);
            // Write out the order number
            for (j = 0; j < count; j++) {
                write(fdits[3][1], order_Num_A[j], strlen(order_Num_A[j]));
                write(fdits[3][1], ";", 1);
            }
            // Write out the order start day
            for (j = 0; j < count; j++) {
                write(fdits[3][1], order_Start_Day_Char[j], strlen(order_Start_Day_Char[j]));
                write(fdits[3][1], ";", 1);
            }
            //  Write out the order end day
            for (j = 0; j < count; j++) {
                write(fdits[3][1], order_End_Day_Char[j], strlen(order_End_Day_Char[j]));
                write(fdits[3][1], ";", 1);
            }
            //  Write out the order Quantity
            for (j = 0; j < count; j++) {
                write(fdits[3][1], order_Quantity_A[j], strlen(order_Quantity_A[j]));
                write(fdits[3][1], ";", 1);
            }
            // Write out the plant which produced the orders
            write(fdits[3][1], order_Plant, strlen(order_Plant));
            write(fdits[3][1], ";", 1);
            //  Write out the number of rejected orders
            write(fdits[3][1], rejectROW, strlen(rejectROW));
            write(fdits[3][1], ";", 1);
            // Write out the rejected order number
            for (j = 0; j < rejectCount; j++) {
                write(fdits[3][1], order_Num[rejectedOrder[j]], strlen(order_Num[rejectedOrder[j]]));
                write(fdits[3][1], ";", 1);
            }
            //  Write out the rejected order name
            for  (j = 0; j < rejectCount; j++) {
                write(fdits[3][1], order_Name[rejectedOrder[j]], strlen(order_Name[rejectedOrder[j]]));
                write(fdits[3][1], ";", 1);
            }
            //  Write out the rejected order due
            for (j = 0; j < rejectCount; j++) {
                write(fdits[3][1], order_Due_Char[rejectedOrder[j]], strlen(order_Due_Char[rejectedOrder[j]]));
                write(fdits[3][1], ";", 1);
            }
            // Write out the number of days in use for X,Y,Z
            for (j = 0; j < 3; j++) {
                write(fdits[3][1], NumOfDaysInUse[j], strlen(NumOfDaysInUse[j]));
                write(fdits[3][1], ";", 1);
            }
            // Write out the total work of X,Y and Z
            for (j = 0; j < 3; j++) {
                write(fdits[3][1], plant_Work_SUM_Char[j], strlen(plant_Work_SUM_Char[j]));
                write(fdits[3][1], ";", 1);
            }
            if(endAnalyse!=1){
               write(fdits[3][1], "e;", 2);
            }
            else{
               write(fdits[3][1], "E;", 2); //let analyseterminate
                //write(fdits[3][1], "o;", 2);
               break;
            }
        }
        
    }
}

void analyse(int fdits[6][2]) {
    FILE *fp1=fopen("PLS_Report_G11.txt","a");
    fputs("\n\n***PLS Summary Report***\n\n", fp1);
    //fclose(fp1);
    while (1) {
        int j;
        int orderNum = 0, num = 0, counter = 0, start = 0;
        int count = 0, countFlag = 0, printNow = 0, printCount = 0, rejectCount = 0, index = 0, end = 0;
        char report_file[30], buf[128], newBuf[100] = {}, algorithm[10];
        char start_date[11];
        char end_date[11];
        char FIRST_DAY[11] = "xxxx-01-01";
        char order_Num_A[maxOrder][6];
        char order_Start_Day_Char[maxOrder][11];
        char order_End_Day_Char[maxOrder][11];
        char order_Days[maxOrder][5];
        char order_Quantity_A[maxOrder][5];
        char order_Plant[maxOrder];
        char order_Num_reject[maxOrder][6];
        char order_Name_reject[maxOrder][15];
        char order_Due_reject[maxOrder][11];
        char NumOfDaysInUse[3][6];
        char plant_Work_SUM[3][10];
        memset(buf, '\0', sizeof(buf));
        memset(newBuf, '\0', sizeof(newBuf));
        // Start Read in data
        while (1) {
            read(fdits[3][0], buf, 1);
            if (strcmp(buf, ";") == 0) {
                // End reading in data
                if (strcmp(newBuf, "e") == 0) {
                    printNow = 1;
                    break;
                }
                if (strcmp(newBuf, "E") == 0) {
                    printNow = 1;
                    end=1;
                    break;
                }
                //  Read in algorithm type
                if (count == 0) {
                    if (strcmp(newBuf, "FDFSJF") == 0) { strcpy(algorithm, newBuf); }
                    if (strcmp(newBuf, "FCFS") == 0) { strcpy(algorithm, newBuf); }
                    //if (strcmp(newBuf, "o") == 0) { end = 1; break; }
                }
                //  Read in file name
                else if (count == 1) strcpy(report_file, newBuf);
                //  Read in start date and end date
                else if (count == 2) {
                    strncpy(start_date, newBuf, 10);
                    start_date[10] = 0;
                    strncpy(end_date, newBuf + 11, 10);
                    end_date[10] = 0;
                }
                //  Read in the total number of orders
                else if (count == 3) {
                    orderNum = atoi(newBuf);
                }
                //  Read in the total number of rows for printing analyse report
                else if (count == 4) {
                    printCount = atoi(newBuf);
                }
                //  Read in the order number
                else if (count > 4 && count < 5 + printCount) {
                    index = count - 5;
                    strcpy(order_Num_A[index], newBuf);
                }
                //  Read in the order start day
                else if (count > 4 + printCount && count < 5 + 2 * printCount) {
                    index = count - 5 - printCount;
                    strcpy(order_Start_Day_Char[index], newBuf);
                }
                //  Read in the order end day
                else if (count > 4 + 2 * printCount && count < 5 + 3 * printCount) {
                    index = count - 5 - 2 * printCount;
                    strcpy(order_End_Day_Char[index], newBuf);
                }
                //  Read in the order Quantity
                else if (count > 4 + 3 * printCount && count < 5 + 4 * printCount) {
                    index = count - 5 - 3 * printCount;
                    strcpy(order_Quantity_A[index], newBuf);
                }
                //  Read in the plant which produced the orders
                else if (count == 5 + 4 * printCount) {
                    strcpy(order_Plant, newBuf);
                }
                //  Read in the number of rejected orders
                else if (count == 6 + 4 * printCount) {
                    rejectCount = atoi(newBuf);
                }
                //  Read in the rejected order number
                else if (count > 6 + 4 * printCount && count <= 6 + 4 * printCount + rejectCount) {
                    index = count - 7 - 4 * printCount;
                    strcpy(order_Num_reject[index], newBuf);
                }
                //  Read in the rejected order name
                else if (count > 6 + 4 * printCount + rejectCount && count <= 6 + 4 * printCount + 2 * rejectCount) {
                    index = count - 7 - 4 * printCount - rejectCount;
                    strcpy(order_Name_reject[index], newBuf);
                }
                //  Read in the rejected order due
                else if (count > 6 + 4 * printCount + 2 * rejectCount && count <= 6 + 4 * printCount + 3 * rejectCount) {
                    index = count - 7 - 4 * printCount - 2 * rejectCount;
                    strcpy(order_Due_reject[index], newBuf);
                }
                // Read in the number of days in use for X,Y,Z
                else if (count > 6 + 4 * printCount + 3 * rejectCount && count <= 9 + 4 * printCount + 3 * rejectCount) {
                    index = count - 7 - 4 * printCount - 3 * rejectCount;
                    strcpy(NumOfDaysInUse[index], newBuf);
                }
                // Read in the total work of X,Y and Z
                else if (count > 9 + 4 * printCount + 3 * rejectCount && count <= 12 + 4 * printCount + 3 * rejectCount) {
                    index = count - 10 - 4 * printCount - 3 * rejectCount;
                    strcpy(plant_Work_SUM[index], newBuf);
                }
                memset(newBuf, '\0', sizeof(newBuf));
                countFlag = 0;
                count++;
            }
            else {
                newBuf[countFlag] = buf[0];
                countFlag++;
            }
            memset(buf, '\0', sizeof(buf));
        }
        memset(buf, '\0', sizeof(buf));
        memset(newBuf, '\0', sizeof(newBuf));
        printf("\n");
        // BREAK the process if there is a signal comes from the scheduling process
        //if (end) break;
        
        if(!end){
            //printf("not-end\n");
            FILE *fp;
            fp = fopen(report_file, "w");
            //fp2 =fopen("PLS_Report_G11.txt","a");
            fputs("\n", fp);
            for (j = 0; j < 4; j++) FIRST_DAY[j] = start_date[j];
            start = day(FIRST_DAY, start_date);
            fputs("\n***PLS Schedule Analysis Report***\n\n", fp);
            fprintf(fp, "Algorithm used: %s\n\n", algorithm);
            fprintf(fp, "There are %d Orders ACCEPTED. Details are as follows: \n\n", orderNum - rejectCount);
            fputs("ORDER NUMBER          START            END           DAYS       QUANTITY      PLANT\n", fp);
            fputs("======================================================================================\n", fp);
            for (j = 0; j < printCount; j++) {
                fprintf(fp, "%-12s%15s%15s%13d%15s", order_Num_A[j], order_Start_Day_Char[j], order_End_Day_Char[j], day(order_Start_Day_Char[j], order_End_Day_Char[j]), order_Quantity_A[j]);
                if (order_Plant[j] == 'X') fprintf(fp, "%15s ", "Plant_X");
                if (order_Plant[j] == 'Y') fprintf(fp, "%15s ", "Plant_Y");
                if (order_Plant[j] == 'Z') fprintf(fp, "%15s ", "Plant_Z");
                fputs("\n", fp);
            }
            fputs("\n                                -End-\n\n", fp);
            fputs("======================================================================================\n\n\n", fp);
            fprintf(fp, "There are %d Orders REJECTED. Details are as follows: \n\n", rejectCount);
            fputs("ORDER NUMBER   PRODUCT NAME       DUE DATE       QUANTITY\n", fp);
            fputs("======================================================================================\n", fp);
            for (j = 0; j < rejectCount; j++) {
                fprintf(fp, "%-12s%15s%15s        ????\n", order_Num_reject[j], order_Name_reject[j], order_Due_reject[j]);
            }
            fputs("\n                                -End-\n\n", fp);
            fputs("======================================================================================\n\n\n", fp);
            float utilization = 0;
            utilization = (float)atoi(plant_Work_SUM[0]) / (atoi(NumOfDaysInUse[0]) * 300);
            fputs("***PERFORMANCE\n", fp);
            fputs("\nPlant_X: \n", fp);
            fprintf(fp, "             Number of days in use:                    %s days\n", NumOfDaysInUse[0]);
            fprintf(fp, "             Number of products produced:              %s (in total)\n", plant_Work_SUM[0]);
            fprintf(fp, "             Utilization of the plant:                 %.1f %%\n", utilization * 100);
            fputs("\nPlant_Y: \n", fp);
            utilization = (float)atoi(plant_Work_SUM[1]) / (atoi(NumOfDaysInUse[1]) * 400);
            fprintf(fp, "             Number of days in use:                    %s days\n", NumOfDaysInUse[1]);
            fprintf(fp, "             Number of products produced:              %s (in total)\n", plant_Work_SUM[1]);
            fprintf(fp, "             Utilization of the plant:                 %.1f %%\n", utilization * 100);
            fputs("\nPlant_Z: \n", fp);
            utilization = (float)atoi(plant_Work_SUM[2]) / (atoi(NumOfDaysInUse[2]) * 500);
            fprintf(fp, "             Number of days in use:                    %s days\n", NumOfDaysInUse[2]);
            fprintf(fp, "             Number of products produced:              %s (in total)\n", plant_Work_SUM[2]);
            fprintf(fp, "             Utilization of the plant:                 %.1f %%\n", utilization * 100);

            float totalUtilization = (float)(atoi(plant_Work_SUM[0]) + atoi(plant_Work_SUM[1]) + atoi(plant_Work_SUM[2])) / (atoi(NumOfDaysInUse[0]) * 300 + atoi(NumOfDaysInUse[1]) * 400 + atoi(NumOfDaysInUse[2]) * 500);
            fprintf(fp, "\nOverall of utilization: %35.1f %%\n\n\n", totalUtilization * 100);
            fprintf(fp1, "\nOverall of utilization for algorithm %s: %35.1f %%\n\n", algorithm, totalUtilization * 100);
            fclose(fp);
            
            char totalUtilization_c[5];
            gcvt(totalUtilization * 100, 3, totalUtilization_c);
            //printf("utilization is %s\n", totalUtilization_c);

            write(fdits[5][1], algorithm, strlen(algorithm));
            write(fdits[5][1], totalUtilization_c, strlen(totalUtilization_c));

            write(fdits[4][1], "OK", 2);
        }
        if (end) {
            //FILE *fp;
            //fp =fopen("PLS_Report_G11.txt","a");
            //fp1=fopen("PLS_Report_G11.txt","a");
            //printf("end arived\n");
            fputs("\n", fp1);
            for (j = 0; j < 4; j++) FIRST_DAY[j] = start_date[j];
            start = day(FIRST_DAY, start_date);
            fputs("\n\n***PLS Schedule Analysis Report***\n\n", fp1);
            fprintf(fp1, "Use the best algorithm: %s\n\n", algorithm);
            fprintf(fp1, "There are %d Orders ACCEPTED. Details are as follows: \n\n", orderNum - rejectCount);
            fputs("ORDER NUMBER          START            END           DAYS       QUANTITY      PLANT\n", fp1);
            fputs("======================================================================================\n", fp1);
            for (j = 0; j < printCount; j++) {
                fprintf(fp1, "%-12s%15s%15s%13d%15s", order_Num_A[j], order_Start_Day_Char[j], order_End_Day_Char[j], day(order_Start_Day_Char[j], order_End_Day_Char[j]), order_Quantity_A[j]);
                if (order_Plant[j] == 'X') fprintf(fp1, "%15s ", "Plant_X");
                if (order_Plant[j] == 'Y') fprintf(fp1, "%15s ", "Plant_Y");
                if (order_Plant[j] == 'Z') fprintf(fp1, "%15s ", "Plant_Z");
                fputs("\n", fp1);
            }
            fputs("\n                                -End-\n\n", fp1);
            fputs("======================================================================================\n\n\n", fp1);
            fprintf(fp1, "There are %d Orders REJECTED. Details are as follows: \n\n", rejectCount);
            fputs("ORDER NUMBER   PRODUCT NAME       DUE DATE       QUANTITY\n", fp1);
            fputs("======================================================================================\n", fp1);
            for (j = 0; j < rejectCount; j++) {
                fprintf(fp1, "%-12s%15s%15s        ????\n", order_Num_reject[j], order_Name_reject[j], order_Due_reject[j]);
            }
            fputs("\n                                -End-\n\n", fp1);
            fputs("======================================================================================\n\n\n", fp1);
            float utilization = 0;
            utilization = (float)atoi(plant_Work_SUM[0]) / (atoi(NumOfDaysInUse[0]) * 300);
            fputs("***PERFORMANCE\n", fp1);
            fputs("\nPlant_X: \n", fp1);
            fprintf(fp1, "             Number of days in use:                    %s days\n", NumOfDaysInUse[0]);
            fprintf(fp1, "             Number of products produced:              %s (in total)\n", plant_Work_SUM[0]);
            fprintf(fp1, "             Utilization of the plant:                 %.1f %%\n", utilization * 100);
            fputs("\nPlant_Y: \n", fp1);
            utilization = (float)atoi(plant_Work_SUM[1]) / (atoi(NumOfDaysInUse[1]) * 400);
            fprintf(fp1, "             Number of days in use:                    %s days\n", NumOfDaysInUse[1]);
            fprintf(fp1, "             Number of products produced:              %s (in total)\n", plant_Work_SUM[1]);
            fprintf(fp1, "             Utilization of the plant:                 %.1f %%\n", utilization * 100);
            fputs("\nPlant_Z: \n", fp1);
            utilization = (float)atoi(plant_Work_SUM[2]) / (atoi(NumOfDaysInUse[2]) * 500);
            fprintf(fp1, "             Number of days in use:                    %s days\n", NumOfDaysInUse[2]);
            fprintf(fp1, "             Number of products produced:              %s (in total)\n", plant_Work_SUM[2]);
            fprintf(fp1, "             Utilization of the plant:                 %.1f %%\n", utilization * 100);

            float totalUtilization = (float)(atoi(plant_Work_SUM[0]) + atoi(plant_Work_SUM[1]) + atoi(plant_Work_SUM[2])) / (atoi(NumOfDaysInUse[0]) * 300 + atoi(NumOfDaysInUse[1]) * 400 + atoi(NumOfDaysInUse[2]) * 500);
            fprintf(fp1, "\nOverall of utilization: %35.1f %%\n\n\n", totalUtilization * 100);
            fclose(fp1);
            break;
        }
        
        
        
    /**    FILE *fp, *fp2;
        if(!end){
            fp = fopen(report_file, "w");
            fp2 =fopen("PLS_Report_G11.txt","a");
        }
        if(end) fp=fopen("PLS_Report_G11.txt","a");
        fputs("\n", fp);
        for (j = 0; j < 4; j++) FIRST_DAY[j] = start_date[j];
        start = day(FIRST_DAY, start_date);
        if(!end) fputs("\n***PLS Schedule Analysis Report***\n\n", fp);
        if(!end) fprintf(fp, "Algorithm used: %s\n\n", algorithm);
        if(end) fprintf(fp, "\nUse a best algorithm: %s\n\n", algorithm);
        fprintf(fp, "There are %d Orders ACCEPTED. Details are as follows: \n\n", orderNum - rejectCount);
        fputs("ORDER NUMBER          START            END           DAYS       QUANTITY      PLANT\n", fp);
        fputs("======================================================================================\n", fp);
        for (j = 0; j < printCount; j++) {
            fprintf(fp, "%-12s%15s%15s%13d%15s", order_Num_A[j], order_Start_Day_Char[j], order_End_Day_Char[j], day(order_Start_Day_Char[j], order_End_Day_Char[j]), order_Quantity_A[j]);
            if (order_Plant[j] == 'X') fprintf(fp, "%15s ", "Plant_X");
            if (order_Plant[j] == 'Y') fprintf(fp, "%15s ", "Plant_Y");
            if (order_Plant[j] == 'Z') fprintf(fp, "%15s ", "Plant_Z");
            fputs("\n", fp);
        }
        fputs("\n                                -End-\n\n", fp);
        fputs("======================================================================================\n\n\n", fp);
        fprintf(fp, "There are %d Orders REJECTED. Details are as follows: \n\n", rejectCount);
        fputs("ORDER NUMBER   PRODUCT NAME       DUE DATE       QUANTITY\n", fp);
        fputs("======================================================================================\n", fp);
        for (j = 0; j < rejectCount; j++) {
            fprintf(fp, "%-12s%15s%15s        ????\n", order_Num_reject[j], order_Name_reject[j], order_Due_reject[j]);
        }
        fputs("\n                                -End-\n\n", fp);
        fputs("======================================================================================\n\n\n", fp);
        float utilization = 0;
        utilization = (float)atoi(plant_Work_SUM[0]) / (atoi(NumOfDaysInUse[0]) * 300);
        fputs("***PERFORMANCE\n", fp);
        fputs("\nPlant_X: \n", fp);
        fprintf(fp, "             Number of days in use:                    %s days\n", NumOfDaysInUse[0]);
        fprintf(fp, "             Number of products produced:              %s (in total)\n", plant_Work_SUM[0]);
        fprintf(fp, "             Utilization of the plant:                 %.1f %%\n", utilization * 100);
        fputs("\nPlant_Y: \n", fp);
        utilization = (float)atoi(plant_Work_SUM[1]) / (atoi(NumOfDaysInUse[1]) * 400);
        fprintf(fp, "             Number of days in use:                    %s days\n", NumOfDaysInUse[1]);
        fprintf(fp, "             Number of products produced:              %s (in total)\n", plant_Work_SUM[1]);
        fprintf(fp, "             Utilization of the plant:                 %.1f %%\n", utilization * 100);
        fputs("\nPlant_Z: \n", fp);
        utilization = (float)atoi(plant_Work_SUM[2]) / (atoi(NumOfDaysInUse[2]) * 500);
        fprintf(fp, "             Number of days in use:                    %s days\n", NumOfDaysInUse[2]);
        fprintf(fp, "             Number of products produced:              %s (in total)\n", plant_Work_SUM[2]);
        fprintf(fp, "             Utilization of the plant:                 %.1f %%\n", utilization * 100);

        float totalUtilization = (float)(atoi(plant_Work_SUM[0]) + atoi(plant_Work_SUM[1]) + atoi(plant_Work_SUM[2])) / (atoi(NumOfDaysInUse[0]) * 300 + atoi(NumOfDaysInUse[1]) * 400 + atoi(NumOfDaysInUse[2]) * 500);
        fprintf(fp, "\nOverall of utilization: %35.1f %%\n\n\n", totalUtilization * 100);
        if (!end) {
            fprintf(fp2, "\nOverall of utilization for algorithm %s: %35.1f %%\n\n", algorithm, totalUtilization * 100);
            fclose(fp);
            fclose(fp2);
        }
        if (end) {
            fclose(fp);
            break;
        }**/
        
            

    }
}

void output(int fdits[6][2]) {
    while (1) {
        int j;
        int orderNum = 0, num = 0, counter = 0, start = 0;
        int count = 0, countFlag = 0, printNow = 0, index = 0, end = 0;
        char report_file[30], buf[128], newBuf[100] = {}, algorithm[5];
        char start_date[11];                  /* period stores the start date in string format */
        char end_date[11];                     /* period stores the end date in string format */
        char FIRST_DAY[11] = "xxxx-01-01";     /* First day of some year */
        char  plant_X_Work_Char[maxOrder][5];
        char  plant_Y_Work_Char[maxOrder][5];
        char  plant_Z_Work_Char[maxOrder][5];

        char  plant_X_OrderName[maxOrder][10];
        char  plant_Y_OrderName[maxOrder][10];
        char  plant_Z_OrderName[maxOrder][10];

        char  plant_X_OrderNum[maxOrder][6];
        char  plant_Y_OrderNum[maxOrder][6];
        char  plant_Z_OrderNum[maxOrder][6];

        char  plant_X_DueDate[maxOrder][11];
        char  plant_Y_DueDate[maxOrder][11];
        char  plant_Z_DueDate[maxOrder][11];
        char plant_Work_SUM[3][10];
        int NumOfDaysInUse[3];
        memset(buf, '\0', sizeof(buf));
        memset(newBuf, '\0', sizeof(newBuf));
        //close(0);
            //dup2(fdits[2][0], 0);
        while (1) {
            read(fdits[2][0], buf, 1);
            if (strcmp(buf, ";") == 0) {
                // End reading in data
                if (strcmp(newBuf, "e") == 0) {
                    printNow = 1;
                    break;
                }
                if (strcmp(newBuf, "E") == 0) {
                    printNow = 1;
                    end =1;
                    break;
                }
                //  Read in algorithm type
                if (count == 0) {
                    if (strcmp(newBuf, "FDFSJF") == 0) { strcpy(algorithm, newBuf); }//end = 1; }
                    if (strcmp(newBuf, "FCFS") == 0) { strcpy(algorithm, newBuf); }
                }
                //  Read in start date and end date
                else if (count == 1) {
                    strncpy(start_date, newBuf, 10);
                    start_date[10] = 0;
                    strncpy(end_date, newBuf + 11, 10);
                    end_date[10] = 0;
                }
                // Read in the number of days in use for X,Y,Z
                else if (count >= 2 && count < 5) { NumOfDaysInUse[count - 2] = atoi(newBuf); }
                // Read in order names of orders which plant X finished
                else if (count >= 5 && count < 5 + NumOfDaysInUse[0]) {
                    strcpy(plant_X_OrderName[count - 5], newBuf);
                }
                // Read in order number of orders which plant X finished
                else if (count >= 5 + NumOfDaysInUse[0] && count < 5 + 2 * NumOfDaysInUse[0]) {
                    index = count - 5 - NumOfDaysInUse[0];
                    strcpy(plant_X_OrderNum[index], newBuf);
                }
                // Read in quantity of orders which plant X worked
                else if (count >= 5 + 2 * NumOfDaysInUse[0] && count < 5 + 3 * NumOfDaysInUse[0]) {
                    index = count - 5 - 2 * NumOfDaysInUse[0];
                    strcpy(plant_X_Work_Char[index], newBuf);
                }
                // Read in due day of orders which plant X finished
                else if (count >= 5 + 3 * NumOfDaysInUse[0] && count < 5 + 4 * NumOfDaysInUse[0]) {
                    index = count - 5 - 3 * NumOfDaysInUse[0];
                    strcpy(plant_X_DueDate[index], newBuf);
                }

                // Read in order names of orders which plant Y finished
                else if (count >= 5 + 4 * NumOfDaysInUse[0] && count < 5 + NumOfDaysInUse[1] + 4 * NumOfDaysInUse[0]) {
                    index = count - 5 - 4 * NumOfDaysInUse[0];
                    strcpy(plant_Y_OrderName[index], newBuf);
                }
                // Read in order number of orders which plant Y finished
                else if (count >= 5 + NumOfDaysInUse[1] + 4 * NumOfDaysInUse[0] && count < 5 + 2 * NumOfDaysInUse[1] + 4 * NumOfDaysInUse[0]) {
                    index = count - 5 - NumOfDaysInUse[1] - 4 * NumOfDaysInUse[0];
                    strcpy(plant_Y_OrderNum[index], newBuf);
                }
                // Read in quantity of orders which plant Y worked
                else if (count >= 5 + 2 * NumOfDaysInUse[1] + 4 * NumOfDaysInUse[0] && count < 5 + 3 * NumOfDaysInUse[1] + 4 * NumOfDaysInUse[0])
                {
                    index = count - 5 - 2 * NumOfDaysInUse[1] - 4 * NumOfDaysInUse[0];
                    strcpy(plant_Y_Work_Char[index], newBuf);
                }
                // Read in due day of orders which plant Y finished
                else if (count >= 5 + 3 * NumOfDaysInUse[1] + 4 * NumOfDaysInUse[0] && count < 5 + 4 * NumOfDaysInUse[1] + 4 * NumOfDaysInUse[0]) {
                    index = count - 5 - 3 * NumOfDaysInUse[1] - 4 * NumOfDaysInUse[0];
                    strcpy(plant_Y_DueDate[index], newBuf);
                }

                // Read in order names of orders which plant Z finished
                else if (count >= 5 + 4 * NumOfDaysInUse[1] + 4 * NumOfDaysInUse[0] && count < 5 + NumOfDaysInUse[2] + 4 * NumOfDaysInUse[1] + 4 * NumOfDaysInUse[0]) {
                    index = count - 5 - 4 * NumOfDaysInUse[1] - 4 * NumOfDaysInUse[0];
                    strcpy(plant_Z_OrderName[index], newBuf);
                }
                // Read in order number of orders which plant Z finished
                else if (count >= 5 + NumOfDaysInUse[2] + 4 * NumOfDaysInUse[1] + 4 * NumOfDaysInUse[0] && count < 5 + 2 * NumOfDaysInUse[2] + 4 * NumOfDaysInUse[1] + 4 * NumOfDaysInUse[0]) {
                    index = count - 5 - NumOfDaysInUse[2] - 4 * NumOfDaysInUse[1] - 4 * NumOfDaysInUse[0];
                    strcpy(plant_Z_OrderNum[index], newBuf);
                }
                // Read in quantity of orders which plant Z worked
                else if (count >= 5 + 2 * NumOfDaysInUse[2] + 4 * NumOfDaysInUse[1] + 4 * NumOfDaysInUse[0] && count < 5 + 3 * NumOfDaysInUse[2] + 4 * NumOfDaysInUse[1] + 4 * NumOfDaysInUse[0]) {
                    index = count - 5 - 2 * NumOfDaysInUse[2] - 4 * NumOfDaysInUse[1] - 4 * NumOfDaysInUse[0];
                    strcpy(plant_Z_Work_Char[index], newBuf);
                }
                // Read in due day of orders which plant Z finished
                else if (count >= 5 + 3 * NumOfDaysInUse[2] + 4 * NumOfDaysInUse[1] + 4 * NumOfDaysInUse[0] && count < 5 + 4 * NumOfDaysInUse[2] + 4 * NumOfDaysInUse[1] + 4 * NumOfDaysInUse[0]) {
                    index = count - 5 - 3 * NumOfDaysInUse[2] - 4 * NumOfDaysInUse[1] - 4 * NumOfDaysInUse[0];
                    strcpy(plant_Z_DueDate[index], newBuf);
                }
                memset(newBuf, '\0', sizeof(newBuf));
                countFlag = 0;
                count++;
            }
            else {
                newBuf[countFlag] = buf[0];
                countFlag++;
            }
            memset(buf, '\0', sizeof(buf));
        }

        // Print output.
        if (printNow) {
            printf("\n\nPlant_X (300 per day)\n");
            printf("%s to %s\n\n", start_date, end_date);
            printf("Date           Product Name   Order Number   Quantity       Due Date\n");
            printf("                                            (Produced)\n");
            for (j = 0; j < 4; j++) FIRST_DAY[j] = start_date[j];
            start = day(FIRST_DAY, start_date);
            int period = day(start_date, end_date);
            char NA[] = "NA";
            for (j = 0; j < period; j++) {
                char date[11];
                strcpy(date, start_date);
                IntToDay(date, start, j);
                if (j < NumOfDaysInUse[0]) {
                    printf("%-15s%-15s%-15s%-15s%-15s\n", date, plant_X_OrderName[j], plant_X_OrderNum[j], plant_X_Work_Char[j], plant_X_DueDate[j]);
                }

                else {
                    printf("%-15s%-15s%-15s%-15s%-15s\n", date, NA, NA, NA, NA);
                }
            }
            printf("\n\nPlant_Y (400 per day)\n");
            printf("%s to %s\n\n", start_date, end_date);
            printf("Date           Product Name   Order Number   Quantity       Due Date\n");
            printf("                                            (Produced)\n");
            for (j = 0; j < period; j++) {
                char date[11];
                //char NA[2] = "NA";
                strcpy(date, start_date);
                IntToDay(date, start, j);
                if (j < NumOfDaysInUse[1]) {
                    printf("%-15s%-15s%-15s%-15s%-15s\n", date, plant_Y_OrderName[j], plant_Y_OrderNum[j], plant_Y_Work_Char[j], plant_Y_DueDate[j]);
                }

                else {
                    printf("%-15s%-15s%-15s%-15s%-15s\n", date, NA, NA, NA, NA);
                }
            }
            printf("\n\nPlant_Z (500 per day)\n");
            printf("%s to %s\n\n", start_date, end_date);
            printf("Date           Product Name   Order Number   Quantity       Due Date\n");
            printf("                                          (Produced)\n");
            for (j = 0; j < period; j++) {
                char date[11];
                //char NA[2] = "NA";
                strcpy(date, start_date);
                IntToDay(date, start, j);
                if (j < NumOfDaysInUse[2]) {
                    printf("%-15s%-15s%-15s%-15s%-15s\n", date, plant_Z_OrderName[j], plant_Z_OrderNum[j], plant_Z_Work_Char[j], plant_Z_DueDate[j]);
                }

                else {
                    printf("%-15s%-15s%-15s%-15s%-15s\n", date, NA, NA, NA, NA);
                }
            }
            printf("\n\n");
            write(fdits[4][1], "OK", 2);
        }
        if (end) {break;}
        //break;
    }
    int i;
    for (i = 0; i < 6; i++) {
        close(fdits[i][1]);
        close(fdits[i][0]);
    }
}

/*
    extract <int> number in s from start to start+len-1
*/
int extract_int(char *s, int start, int len) {
    char ss[5];
    strncpy(ss, s + start, len);
    ss[len] = 0;
    return atoi(ss);
}

/*
    leap year: return 1
    common year: return 0
*/
int year(int yr) {
    if ((yr % 400 == 0) || (yr % 100 != 0 && yr % 4 == 0))
        return 1;
    else
        return 0;
}

/*
    return the number of dates in a specific month
*/
int month(int yr, int mth) {
    int days[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (mth != 2)
        return days[mth];
    else
        return days[mth] + year(yr);
}

/*
    return the number of dates between s1 and s2
    if the date is illegal, return -1
*/
int day(char *s1, char *s2) {
    // The format of s1,s2 is "xxxx-xx-xx" [year-month-day].

    int yr1, mth1, dy1; // store the date extracted from s1
    int yr2, mth2, dy2; // store the date extracted from s2
    int d_yr = 0, d_mth = 0, d_dy = 0; // store the difference between date1 and date2

    // extrat date in s1 and s2
    yr1 = extract_int(s1, 0, 4);
    mth1 = extract_int(s1, 5, 2);
    dy1 = extract_int(s1, 8, 2);

    yr2 = extract_int(s2, 0, 4);
    mth2 = extract_int(s2, 5, 2);
    dy2 = extract_int(s2, 8, 2);

    // error handling 1
    if ((yr1 > yr2) || (yr1 == yr2 && mth1 > mth2) || (yr1 == yr2 && mth1 == mth2 && dy1 > dy2)) // date2 is earlier than date1, illegal
        return -1;

    // error handling 2
    if (mth1 > 12 || mth2 > 12 || month(yr1, mth1) < dy1 || month(yr2, mth2) < dy2) // the date not exist
        return -1;

    int i;

    // number of years between date1 and date2
    for (i = yr1 + 1; i <= yr2 - 1; i++) {
        d_yr = d_yr + 365 + year(i);
    }

    // number of months between date1 and date2
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

    // number of days between date1 and date2
    if (yr1 == yr2 && mth1 == mth2) {
        d_dy = dy2 - dy1 + 1;
    }
    else {
        d_dy = -dy1 + 1 + dy2;
    }

    return d_yr + d_mth + d_dy;

}

/*
*    Change integer type variable to string type variable
*
*/
char *IntToString(int num, char *str) {
    int length = 0;
    /*if (num < 0){
        num = -num;
        str[length] = '-';
        length++;
    }*/
    do {
        str[length] = num % 10 + 48;
        length++;
        num /= 10;
    } while (num != 0);

    int j = 0;
    /*if (str[0] == '-'){
        j = 1;
    }*/
    str[length] = '\0';
    for (; j < length / 2; j++) {
        char temp;
        temp = str[j];
        str[j] = str[length - 1 - j];
        str[length - 1 - j] = temp;
    }
    return str;
}
/*
*
*    Change an integer (1~365/366) to a date with format 'xxxx-xx-xx' in type string.
*/
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

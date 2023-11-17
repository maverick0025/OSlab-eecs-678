#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <errno.h>
#include <strings.h>
#include <fcntl.h>

char buffer[1024];
char *args[256];
int numArgs = 0;
int runInBackground = 0;
int numBackgroundJobs = 0;
char thing[1025];

struct Process {
    int id;
    int pid;
    char command[1025];
};

struct Process currentBackgroundJobs[256];

void handlingCommands(char **commandstr, int* backgroundIsActiveArray);
void handlingCommandsWithRedirects(char **commandstr, int* backgroundIsActiveArray);
void executePipes(char **args, int numArgs, int numPipes, int* arr);

void replace_with_env(char **arg) {
    if ((*arg)[0] == '$') {
        char *env_name_ending = strpbrk(*arg, "/ \n\t"); 
        char *rem = NULL;
        
        if (env_name_ending) {
            rem = strdup(env_name_ending);  
            *env_name_ending = '\0';  
        }

        char *env_value = getenv(*arg + 1);
        if (env_value) {
            char *new_arg = (char *) malloc(strlen(env_value) + (rem ? strlen(rem) : 0) + 1);
            strcpy(new_arg, env_value);
            if (rem) {
                strcat(new_arg, rem);
                free(rem);
            }
            strcpy(*arg, new_arg);
            // *arg = strdup(new_arg);
            free(new_arg);
        } else {
            *arg = rem ? rem : ""; 
            free(rem);
        }
    	
    }
}

void handleMultiplePipesandhandlingCommands(char **args, int n, int* backgroundIsActiveArray){

    int numPipes =0;

    for (int i =0; i<n; i++){
        if(strcmp(args[i], "|") == 0){
            numPipes++;
        }
    }

    // printf("%d", numPipes);

    if (numPipes <= 0) {
        handlingCommandsWithRedirects(args, backgroundIsActiveArray);
    }

    else {
        executePipes(args, n, numPipes, backgroundIsActiveArray);
    }

}

void executePipes(char **args, int numArgs, int numPipes, int* arr) {
    //arrange commands
    char ***commands = malloc((numPipes + 1) * sizeof(char **));

    int cmdIndex  = 0;

    commands[cmdIndex++] = args;
    for(int i =0; i< numArgs; i++){
       if(strcmp(args[i], "|") == 0){
        args[i] = NULL;
        commands[cmdIndex++] = &args[i+1];
       }
        
    }
    
    /*
    for (int i = 0; i < numPipes + 1; i++) {
        printf("Command %d ", i+1);
        for (int j = 0; commands[i][j] != NULL; j++) {
            printf("%s ", commands[i][j]);
        }
        printf("\n");
    }
    */

    fflush(stdout);

    int n = numPipes+1;
    int pp[n-1][2]; //hold pipes in this
    pid_t pids[n];
    int status;
   
    //create pipes
    for(int i =0; i< n-1; i++){
        if(pipe(pp[i]) == -1){
            perror("Error creating the pipe");
            exit(1);
        }
    }

    //childs are in for loop
    for(int i =0; i< n ; i++){

        pids[i] = fork();

        if( pids[i] == -1 ){
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if(pids[i] == 0 ){

            if(i != 0) {
                dup2(pp[i-1][0], STDIN_FILENO);
                close(pp[i-1][0]);
            }

            if( i != n-1 ) {
                dup2(pp[i][1], STDOUT_FILENO);
                close(pp[i][1]);
            }

            for(int j = 0; j< n-1; j++){
                close(pp[j][0]);
                close(pp[j][1]);
            }

            handlingCommandsWithRedirects(commands[i], arr);
            free(commands);
            exit(0);
        }

    }
    free(commands);

    for(int i =0;  i< numPipes; i++){
        close(pp[i][0]);
        close(pp[i][1]);
    }


    //Parent is here
    for(int i=0; i<numPipes+1; i++){
        waitpid(pids[i], &status, 0);
    } 
}

void handlingCommandsWithRedirects(char **commandstr, int* backgroundIsActiveArray) {
    int rd_in = -1;
    int rd_out = -1;
    int rd_out_app = -1;

    for (int i = 0; commandstr[i]; i++) {
        if (strcmp(commandstr[i], "<") == 0) {
            rd_in = i;
        }
        else if (strcmp(commandstr[i], ">") == 0) {
            rd_out = i;
        }
        else if (strcmp(commandstr[i], ">>") == 0) {
            rd_out_app = i;
        }
    }

    // printf("%d %d ", rd_in, rd_out);

    char* temp[1024];
    if (rd_in == -1 && rd_out == -1 && rd_out_app == -1) {
        int i = 0;
        for (i; commandstr[i]; i++) {
            temp[i] = commandstr[i];
            // printf("%s ", temp[i]);
        }
        temp[i] = NULL;
    }
    else {
        int i = 0;
        for (i; commandstr[i] && strcmp(commandstr[i], ">") != 0 && strcmp(commandstr[i], "<") != 0 && strcmp(commandstr[i], ">>") != 0; i++) {
            temp[i] = commandstr[i];
            // printf("%s ", temp[i]);
        }
        temp[i] = NULL;
    }
    
    int status;
    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) {
        if (rd_in != -1) {
            int infd = open(commandstr[rd_in+1], O_RDONLY);
            dup2(infd, STDIN_FILENO);
            close(infd);
        }

        if (rd_out != -1) {
            int outfd = open(commandstr[rd_out+1],  O_CREAT | O_WRONLY | O_TRUNC, 0666);
            dup2(outfd, STDOUT_FILENO);
            close(outfd);
        }

        if (rd_out_app != -1) {
            int outfd = open(commandstr[rd_out_app+1],  O_CREAT | O_WRONLY | O_APPEND, 0666);
            dup2(outfd, STDOUT_FILENO);
            close(outfd);
        }

        handlingCommands(temp, backgroundIsActiveArray);
        exit(0);
    }
    else {
        waitpid(pid, &status, 0);
    }
}

void handlingCommands(char **commandstr, int* backgroundIsActiveArray)
{
    int exitStatus;
    int pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        if (strcmp("pwd", commandstr[0]) == 0)
        {
            char cwd[1024];
            getcwd(cwd, 1024);
            printf("%s\n", cwd);
        }

        else if (strcmp("ls", commandstr[0]) == 0)
        {
            execvp("/bin/ls", commandstr);
        }

        else if (strcmp("echo", commandstr[0]) == 0)
        {
            for (int i = 0; commandstr[i]; i++) {
                replace_with_env(&commandstr[i]);
            }
        
            for (int i = 1; commandstr[i]; i++) {
                printf("%s ", commandstr[i]);
            }
            printf("\n");
        }
        
        else if (strcmp("du",  commandstr[0]) ==0 ){
	        for (int i = 0; commandstr[i]; i++) {
                replace_with_env(&commandstr[i]);
            }
            execvp(commandstr[0], commandstr);
        }

        else if (strcmp("kill", commandstr[0]) == 0) {
            // free(thing);
            // kill(pid, sig), user wiill give as kill sig pid
            int res = kill(atoi(commandstr[2]), atoi(commandstr[1]));
            for (int i = 0; i < numBackgroundJobs; i++) {
                if (currentBackgroundJobs[i].pid == atoi(commandstr[2])) {
                    backgroundIsActiveArray[i] = 0;
                }
            }
        }

        else if (strcmp("sleep", commandstr[0]) == 0) {
            sleep(atoi(commandstr[1]));
        }

        else {
            execvp(commandstr[0], commandstr);
            // this line should only be reached if exec fails
            printf("Command or executable '%s' not found\n", commandstr[0]);
        }

        exit(0);
    }
    else
    {
        waitpid(pid, &exitStatus, 0);
    }
}

void handlingCommandsBackground(char** commandstr, char* cmd, int* backgroundIsActiveArray) {
    int pid = fork();

    int numPipes =0;

    for (int i =0; i<numArgs; i++){
        if(strcmp(args[i], "|") == 0){
            numPipes++;
        }
    }

    // printf("%d", numPipes);

    if (pid < 0) {
        fprintf(stderr, "Fork failed\n");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {

        int index = numBackgroundJobs;
        printf("\nBackground job started: [%d] %d %s", numBackgroundJobs+1, getpid(), cmd); 

        if (numPipes <= 0) {
            handlingCommandsWithRedirects(commandstr, backgroundIsActiveArray);
            printf("\nCompleted: [%d] %d %s\n", index+1, getpid(), cmd);
            backgroundIsActiveArray[numBackgroundJobs] = 0;
            exit(0);
        }
        else {
            executePipes(commandstr, numArgs, numPipes, backgroundIsActiveArray);
            printf("\nCompleted: [%d] %d %s\n", index+1, getpid(), cmd);
            backgroundIsActiveArray[numBackgroundJobs] = 0;
            exit(0);
        }
    }
    else
    {
        sleep(1);
        currentBackgroundJobs[numBackgroundJobs].pid = pid;
        strcpy(currentBackgroundJobs[numBackgroundJobs].command, cmd);
        // currentBackgroundJobs[numBackgroundJobs].command = strdup(cmd);
        currentBackgroundJobs[numBackgroundJobs].id = numBackgroundJobs+1;
        numBackgroundJobs++;
    }    
}

// TODO: add case for initializing arbitrary variable without assignment
int export(char** commandstr) {
    if (strchr(commandstr[1], '$') == NULL) {
        // assumes valid input
        // eg export testvar=testval
        char* e = strchr(commandstr[1], '=');
        int index = (int)(e - commandstr[1]);
        
        char* temp1 = malloc(index+1);
        strncpy(temp1, commandstr[1], index);
        temp1[index] = 0;

        setenv(temp1, commandstr[1]+index+1, 1);
        
        free(temp1);
    } else { 
        // assumes valid input
        // eg export PATH=$HOME
        char* e = strchr(commandstr[1], '=');
        int index = (int)(e - commandstr[1]);
        
        char* temp1 = malloc(index+1);
        strncpy(temp1, commandstr[1], index);
        temp1[index] = 0;

        setenv(temp1, getenv(commandstr[1]+index+2), 1);
        
        free(temp1);
    }

    return 0;
}

void changeDir(char** commandstr) {

    for (int i = 0; commandstr[i]; i++) {
        replace_with_env(&commandstr[i]);
    }

    if (chdir(commandstr[1]) != 0) {
        printf("%s is not a directory\n", commandstr[1]);
    }

    char cwd[1024];
    getcwd(cwd, 1024);
    setenv("PWD", cwd, 1);
}

void tokenize(char *command)
{
    memset(args, 0, sizeof(args));
    char *token;
    token = strtok(command, " \n\"'");

    while (token != NULL)
    {
        // comments
        if (strcmp(token, "#") == 0 || token[0] == '#') {
            return;
        } 
        if (strcmp(token, "&") == 0) {
            runInBackground = 1;
            token = strtok(NULL, " \n\"'");
        }
        else {
            args[numArgs] = token;
            token = strtok(NULL, " \n\"'");
            numArgs++;
        }
    }

    args[numArgs] = NULL;
}

void jobs(int* backgroundIsActiveArray) {
    for (int i=0; i < numBackgroundJobs; i++) {
        if (backgroundIsActiveArray[i] == 1) {
            printf("[%d] %d %s\n", currentBackgroundJobs[i].id, currentBackgroundJobs[i].pid, currentBackgroundJobs[i].command);
        }
    }
}

int main()
{
    /* identifier for the shared memory segment */
    int segment_id;
    /* pointer to the shared memory segment */
    int *shared_buf;
    /* size of the shared memory segment */
    int size;
    /* allocate a shared memory segment */
    size = sizeof(int) * 256;
    segment_id = shmget(IPC_PRIVATE, size, S_IRUSR|S_IWUSR);
    /* attach the shared memory segment */
    shared_buf = (int *) shmat(segment_id, NULL, 0);
    printf("Welcome...\n");

    while (1)
    {
        numArgs = 0;
        memset(buffer, 0, strlen(buffer));
        printf("[Quash]$ ");
        fgets(buffer, 1024, stdin);
        // thing = strdup(buffer);
        strcpy(thing, buffer);
        tokenize(buffer);

        if (args[0] == NULL) {
            continue;
        }

        else if (runInBackground) {
            runInBackground = 0;
            shared_buf[numBackgroundJobs] = 1;
            handlingCommandsBackground(args, thing, shared_buf);
        }

        else 
        {
            if (strstr(args[0], "quash") != NULL) {
                printf("Running quash within quash is not allowed!\n");
            }
            else if (strcmp("exit", args[0]) == 0 || strcmp("quit", args[0]) == 0) { 
                // free(thing);
                exit(0);
            }
            else if (strcmp("export", args[0]) == 0) {
                export(args);
            }
            else if (strcmp("cd", args[0]) == 0) {
                changeDir(args);
            }
            else if (strcmp("jobs", args[0]) == 0) {
                jobs(shared_buf);
            }
            else
            {
                handleMultiplePipesandhandlingCommands(args, numArgs, shared_buf);
                //handlingCommands(args, shared_buf);
            }
        }
        numArgs =0;
        // free(thing);
        memset(buffer, 0, strlen(buffer));
    }

    /* detach the shared memory segment */
    shmdt(shared_buf);
    /* remove the shared memory segment */
    shmctl(segment_id, IPC_RMID, NULL);
    return 0;
}

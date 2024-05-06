#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_COMMANDS 100
#define MAX_COMMAND_LENGTH 250
#define MAX_HISTORY_SIZE 20

char history[MAX_HISTORY_SIZE][MAX_COMMAND_LENGTH];
char line[MAX_COMMAND_LENGTH];

// Serhat Sarı 150200068

// This shell can not handle commands with "|" in it.
// For example this shell can run: ls ; cat f1.txt ; cat f2.txt; history; 
// or can run: cd .. ; quit ; ls

int history_count = 0;

void add_to_history(char *command) {

    if (history_count == MAX_HISTORY_SIZE) {
        for (int i = 1; i < MAX_HISTORY_SIZE; i++) {
            strcpy(history[i - 1], history[i]);
        }
        strcpy(history[MAX_HISTORY_SIZE - 1], command);
    }
    else {
        strcpy(history[history_count], command);
        history_count++;
    }
}

void display_history() {

    printf("Command History:\n");
    for (int i = 0; i < history_count; i++) {
        printf("%d. %s\n", i + 1, history[i]);
    }
}

void parseLine(char *commands[]){ // Groups up the commands on the line according to "|" delimiter.
                                // For example -> ls ; cat f1.txt | grep hw ; grep odev
                                // { {"ls","cat f1.txt"}, {"grep hw","grep odev"} }
    int command_number = 0;
    commands[command_number] = strtok(line, "|");

    while (commands[command_number] != NULL) {
        commands[++command_number] = strtok(NULL,"|");
    }
}

int countSpaces(char *string){ // Counts spaces in the single element of the grouped commands,
                            // To get how many commands should run concurrently.
    int count = 0;
    for(int i = 0; string[i] != '\0'; i++){
        if(string[i] == ' ')
            count++;
    }
    return count;
}

void parseCommand(char* command, char** argv){ // Parses the command and configures argv array.

    add_to_history(command);
    char *token;
    int number = countSpaces(command);

    if(number == 0){
        argv[0] = command;
        argv[1] = NULL;
    }

    else{
        int count = 0;
        token = strtok(command, " ");
        while (token != NULL) {
            argv[count] = token;
            count++;
            token = strtok(NULL," ");
        }
        argv[count] = NULL;
    }
}

void execNoPipe(char* array[]){

    char* commands[MAX_COMMANDS];
    int index = 0;
    char* token = strtok(array[0], ";");

    while(token != NULL){
        commands[index] = token;
        token = strtok(NULL, ";");
        index++;
    }

    pid_t pid[index];
    int quit = 0;

    for(int i = 0; i<index;i++){
        int number = countSpaces(commands[i]);
        char* arguments[number+1]; 
        parseCommand(commands[i], arguments);

        if(strcmp(arguments[0],"history") == 0){ // Handle history command.
            display_history();
            continue;
        }

        if(!strcmp(arguments[0],"cd")){ // Handle cd command.
            chdir(arguments[1]);
            continue;
        }

        else if(!strcmp(arguments[0], "quit")){ // Raise flag for quit command.
           quit = 1;
        }

        else{
            if((pid[i] = fork()) == 0){ // Fork the child process and enter here.
                execvp(arguments[0],arguments);
                perror("Execvp error."); // Shouldnt enter here.
                _exit(1);
            }
            if(pid[i] < 0){ // Shouldnt enter here.
                perror("Fork error.");
            }
        }

        for(int i = 0 ; i<index;i++){ // For loop to wait for all the child processes to end before moving to
                                    // read the new lines from the batch line.
            if(pid[i]>0){
                int status;
                waitpid(pid[i],&status,0);
            }
        }
    }
    if(quit == 1) // If flag for quit command is raised exit the program after all child processes 
                // besides quit command finishes execution.
        exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {

    FILE* batch_file;

    if(argc != 2){ // If batch files name isnt given in the command line print error and return.
       fprintf(stderr, "Usage: <filename>");
        return(1);
    }

    char* filename = argv[1];
    batch_file = fopen(filename,"r"); // Open batch file

    if(batch_file == NULL){
        perror("Error opening file!");
        return 1;
    }

    while(fgets(line,MAX_COMMAND_LENGTH,batch_file) != NULL){
        line[strcspn(line, "\r\n")] = 0; // Remove new line from end of the line.
        char* commands[MAX_COMMANDS]; // Array of strings to store grouped up commands
        printf("The line shell will process: %s\n", line);
        parseLine(commands); // Parse the commands and store them in the array.
        int commandSize = 0;

        for(int i = 0 ; i<MAX_COMMANDS; i++){
            if(commands[i] == NULL){
                break;
            }
            commandSize++;
        }
        for(int i = 0 ; i<commandSize;i++){
                if(commandSize == 1){
                    execNoPipe(commands);
                }
                else{
                    // Should execute with pipe, no implementation.
                }
            }
    }
    fclose(batch_file);
    return 0;
}

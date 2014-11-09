/*
 * OSLab2_main.c
 *
 *  Created on: Sep 24, 2014
 *      Author: Ryan R. Yurkanin
 */
/*
 *Program Description
 * The Goal of this program, was to solve 3 problems(A, B, C) and eventually by doing that
 * have a linux shell that can use all of the commands available in the bin folder.
 * We must use linux functions such as fork() wait() and execvp() to do the following:
 * Our program must be able to do IO redirection.
 * Our program must be able to handle piping.
 * Our program doesn't need to be able to handle both at the same time.
 * If doing I/O Redirection our program must have an option to have the child and
 * parent run concurrently.
 * This finished project can handle everything required in problems A,B, and C. 
 */

/*
 * Include Details
 * string.h is required for strtok() and strlen()
 * stdlib.h is required for malloc(), exit(), and free()
 * stdio.h is required for fgets() and fflush()
 * unistd.h is required for file modifiers used in I/O redirection
 * fcntl.h is required for in and out in I/O redirection
 * sys/types.h is required for pid_t
 * sys/uio.h and limits.h required for bug removal
 * sys/stat.h required for status of children.
*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>

/*
 * Macro Description
 * MAX_TOKEN is to limit the max size of each individual command.
 * MAX_HISTORY is to set the rewrite point for the history log.
 * MAX_HOSTNAME is to prevent any kind of strange interaction with a large hostname.
 * MAX_DIRECTORY is to limit any errors with going incredibly deep into a directory.
 * MAX_ARGS sets a liimit on how many arguments can be sent to the bin folder commands
 * SUCCESFUL_EXIT, TRUE, FALSE, and ERROR are all used throughout the program.
 */
#define MAX_TOKEN 500
#define MAX_HISTORY 200
#define MAX_HOST_NAME 300
#define MAX_DIRECTORY 600
#define MAX_ARGS 100
#define SUCCESSFUL_EXIT 1337
#define TRUE 1
#define FALSE 0
#define ERROR -1

/*
 * Struct Description
 * argc is the number of arguments being taken in
 * argv is an array of arguments taken in from the CLI
 */
typedef struct {
    int argc;
    char* argv[MAX_ARGS];
} commandStruct;

/*Function Description(Summary)
 * checkConcurrency() checks for the & symbol, and returns a boolean value.
 * checkSpecialCommands() checks for exit or history.
 * checkForInputRedirection() looks for a '<' and returns a boolean value.
 * checkForOutputRedirection() looks for a '>'and returns a boolean value
 * checkforPipe looks for a '|' and returns a boolean value.
 * modifyHistory adds history to the history array to be printed later if desired.
 */
int checkConcurrency(char*, int);
int checkSpecialCommands(commandStruct*, int, int, char*[MAX_HISTORY]);
int checkForInputRedirection(commandStruct*, int*, char**);
int checkForOutputRedirection(commandStruct*, int*, char**);
int checkForPipe(commandStruct*, int*);
void modifyHistory(char*, char*[MAX_HISTORY], int*);

int main() {
    /*
     * Variable Descriptions
     * buffer is used in fgets();
     * input is the string to be tokenized into small seperate commands
     * historicalinput is a separate string that is the same as input.  Used in modify history.
     * commandtoken is the tokenized output from strtok() on input.
     * commandlength will hold how large the input is
     * hostname will story the computer's name
     * currentdirectory keeps track of the directory the user is in.
     * clihistory is an array filled with all commands user has entered
     * finalcommand is used in checkconcurrency and is the last tokenized command.
     * clihistorycounter is used in the printing of the history array.
     * historyprintcounter is also used in the printing of the history array
     */
    char buffer[256];
    char* input = (char*) malloc(MAX_TOKEN);
    char* historicalinput;
    char* commandtoken = (char*) malloc(MAX_TOKEN);
    char* hostname = (char*) malloc(MAX_HOST_NAME);
    char* currentdirectory = (char*) malloc(MAX_DIRECTORY);
    char* clihistory[MAX_HISTORY];
    char* finalcommand = (char*) malloc(MAX_TOKEN);
    int commandlength = 0;
    int clihistorycounter = 0;
    int historyprintcounter = 0;

    /* Setting the shell up for success!*/
    int shell_running = TRUE;
    system("clear");
    gethostname(hostname, MAX_HOST_NAME);


    do {

        /*
         * Variable Descriptions
         */
        int concurrencyactive = FALSE; //Flag for concurrency
        int outputactive = FALSE; //Flag for output redirection
        int inputactive = FALSE; //Flag for input redirection
        int pipeactive = FALSE; //Flag for piping.
        int exclusivity = FALSE; // If redirection, no piping.
        int pipelocation; //Where to separate commands.
        int pipeports[2]; //Used witih pipe())
        char* inputredirect = malloc(MAX_TOKEN); //input command
        char* outputredirect = malloc(MAX_TOKEN); //output command

        getcwd(currentdirectory, MAX_DIRECTORY); //Getting the directory

        commandStruct created_command; //Creating a command
        commandStruct created_command2; //May be used laster for piping

        //Setting arguments to zero
        created_command.argc = 0;
        created_command2.argc = 0;
        printf("@%s:~%s$", hostname, currentdirectory);

        fflush(stdout); //Basic cleaning and taking input.
        input = fgets(buffer, 256, stdin);
        historicalinput = strdup(input);


        /*This block will parse and store the commands in created_command*/

        commandtoken = strtok(input, " ");
        while (commandtoken != NULL) {

            commandlength = strlen(commandtoken);

            if (commandtoken[commandlength - 1] == '\n') { //get rid of new line

                commandtoken[commandlength - 1] = '\0'; //make a c-string

            }

            created_command.argv[created_command.argc] = commandtoken; //add parsed command
            created_command.argc++; //incrememnting argument counter.

            commandtoken = strtok(NULL, " ");
        }
        created_command.argv[created_command.argc] = NULL; //Ending arguments with NULL for future ease.

        modifyHistory(historicalinput, clihistory, &clihistorycounter); //Adding CLI input to history array.

        /* Getting parameters for concurrency check*/
        commandlength = strlen(created_command.argv[created_command.argc - 1]);
        finalcommand = created_command.argv[created_command.argc - 1];

        concurrencyactive = checkConcurrency(finalcommand, commandlength); //Checking Concurrency

        /*Looking for what the user wants from the Shell*/
        shell_running = checkSpecialCommands(&created_command, historyprintcounter, clihistorycounter, clihistory);

        inputactive = checkForInputRedirection(&created_command, &exclusivity, &inputredirect);

        outputactive = checkForOutputRedirection(&created_command, &exclusivity, &outputredirect);

        if(concurrencyactive == TRUE){ //Removing the & from the line
        
            created_command.argv[created_command.argc-1]= NULL;
            created_command.argc--;
        
        }
        
        if (exclusivity == FALSE) { //If there isn't I/O redirection accept a pipe

            pipeactive = (checkForPipe(&created_command, &pipelocation));

            if (pipeactive == TRUE) { //If there is a pipe in the input from user

                int loopcontrol = pipelocation + 1; //First command after pipe
                created_command.argv[pipelocation] = NULL; //Cut of the left side command

                /*Figuring out where the right side command ends*/
                for (loopcontrol; loopcontrol < created_command.argc; loopcontrol++) {
                    created_command2.argv[created_command2.argc] = created_command.argv[loopcontrol];
                    created_command2.argc++;
                }

                created_command2.argv[created_command2.argc] = NULL; //Cutting off the right side command

            }

        }


        pid_t pid1 = fork(); //New process
        int pid1status;
        
        if (pid1 >= 0) { //If less then zero the fork failed

            if (pid1 == 0) { //if true then this is the child

                if (inputactive == TRUE) { //if true then there was input redirection

                    int newstdin = open(inputredirect, O_RDONLY); //redirects input, if possible
                    dup2(newstdin, 0);
                    close(newstdin);

                }

                if (outputactive == TRUE) { //if true then there was output redirection

                    int newstdout = open(outputredirect, O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO); //redirects output, if possible
                    dup2(newstdout, 1);
                    close(newstdout);

                }

                if (pipeactive == TRUE) { //Pipe the two processes

                    pipe(pipeports); //Make a pipe
                    pid_t pid2 = fork(); //New process
                    int pid2status;

                    if (pid2 >= 0) { //If less then zero then the pipe failed

                        if (pid2 == 0) { //If zero then this is child

                            close(pipeports[1]);
                            dup2(pipeports[0], 0);

                            if ((execvp(created_command2.argv[0], created_command2.argv) == -1)) {

                                printf("%s: Command not found.\n", created_command2.argv[0]);
                                exit(ERROR);

                            }

                        } else { //not child

                            close(pipeports[0]); //Doesn't need to be open
                            dup2(pipeports[1], 1); //Dup2 is best because it is atomic.

                            if ((execvp(created_command.argv[0], created_command.argv) == -1)) {

                                printf("%s: Command not found.\n", created_command.argv[0]);
                                exit(ERROR); //Failure

                            }
                        }

                    } else {
                        printf("Fork for pipe has failed\n"); //Failure
                        exit(ERROR);
                    }
                }else if((execvp(created_command.argv[0], created_command.argv) == -1)){
                    printf("%s: Command not found.\n", created_command.argv[0]);
                    exit(ERROR);
                }

            } else if(concurrencyactive == FALSE){
                
                while ((wait(&pid1status)) != pid1);
            }
        } else {
            printf("Fork has failed.\n"); //Failure
            exit(ERROR);
        }

    } while (shell_running);

    return (SUCCESSFUL_EXIT);
}

/*
 * checkConcurrency() Description
 * The purpose of this function is to parse the final command entered for a
 * & sign.  If it fails to find the & symbol then it simply returns FALSE, but
 * if it does find the & symbol then it returns TRUE.  If it can't do either
 * of these things it simply returns an ERROR.
 */
int checkConcurrency(char* finalcommand, int commandlength) {
    if (commandlength > 0) {
        if (finalcommand[commandlength - 1] == '&') {
            
            return TRUE;
        
        } else {

            return FALSE;
        }
    }

    return ERROR;

}

/*
 * modifyHistory() Description
 * The purpose of this function is too manage and keep track of all the CLI
 * inputs the user gives, incase they want to review all that they have asked of
 * the Shell.  It takes in the input, the history array, and a counter that keeps
 * track of how many history elements have been added.
 * If there are more history elements added then MAX_HISTORY, it just begins
 * to rewrite over itself.
 */
void modifyHistory(char* input, char* clihistory[MAX_HISTORY], int *clihistorycounter) {
    if (input != NULL) {

        if (*clihistorycounter < MAX_HISTORY) {
            clihistory[(int) *clihistorycounter] = strdup(input);
            (*clihistorycounter)++;

        } else { //Handling full history with rewrite
            printf("CLI History is full.  Rewriting from beginning.\n");
            (*clihistorycounter) = 0;
            clihistory[(int) *clihistorycounter] = strdup(input);
        }
    } else {

        printf("Command not found!\n");
    }
}

/*
 * checkSpecialCommands() Description
 * The purpose of this function is to parse out commands that are not stored
 * in the bin folder.  If it finds exit it simply sets the While loop's condition
 * to FALSE.  If it finds History it prints all the inputs entered.  If it finds
 * Easter then it just tells you as it is.
 */
int checkSpecialCommands(commandStruct* created_command, int historyprintcounter, int clihistorycounter, char *clihistory[MAX_HISTORY]) {
    if (strcmp(created_command -> argv[0], "exit") == 0) {
        return FALSE;
    }
    if (strcmp(created_command -> argv[0], "history") == 0) {
        for (historyprintcounter; historyprintcounter < clihistorycounter; historyprintcounter++) {
            printf("[%d] %s", historyprintcounter + 1, clihistory[historyprintcounter]);
        }
        return TRUE;
    }
    if (strcmp(created_command -> argv[0], "easter") == 0) {
        printf("There are no eggs here for you!\n");

        return TRUE;
    }
    return TRUE;
}

/*
 *  checkForInputRedirection() Description
 * The purpose of this function is to check all arguments for a <
 * It takes in a Command, a pointer to an int, and a pointer point to a string
 * It loops through until it finds a < if it doesn't it simply returns FALSE
 * If it does, then it returns all commands before the < and sets exclusivity flag to TRUE
 * It also returns TRUE 
 * Also the fact that it bolds comments is interesting, I just learned that.
 */
int checkForInputRedirection(commandStruct* created_command, int* exclusivity, char** inputredirect) {
    int loopcontrol = 1;

    for (loopcontrol; loopcontrol < created_command -> argc; loopcontrol++) {

        if (*created_command -> argv[loopcontrol] == '<') {
            *inputredirect = strdup(created_command -> argv[loopcontrol + 1]);
            created_command ->argv[loopcontrol] = created_command -> argv[loopcontrol+1];
            *exclusivity = TRUE;
        }
        if(*exclusivity == TRUE){
            created_command -> argv[loopcontrol+1] = '\0';
            return TRUE;
        }
    }
    return FALSE;
}

/*
 * checkForOutputRedirection() Description
 * The purpose of this function is to check all arguments for a >
 * It takes in a Command, a pointer to an int, and a pointer point to a string
 * It loops through until it finds a > if it doesn't it simply returns FALSE
 * If it does, then it returns all commands before the > and sets exclusivity flag to TRUE
 * It also returns TRUE
 */
int checkForOutputRedirection(commandStruct* created_command, int* exclusivity, char** outputredirect) {
    int loopcontrol = 1;

    for (loopcontrol; loopcontrol < created_command -> argc; loopcontrol++) {

        if (*created_command -> argv[loopcontrol] == '>') {
            *outputredirect = strdup(created_command -> argv[loopcontrol + 1]);
            created_command -> argv[loopcontrol] = '\0';
            *exclusivity = TRUE;
        }
        if(*exclusivity == TRUE){
            return TRUE;
        }
    }
    return FALSE;
}

/*
 *checkforPipe() Description
 * Takes in a pointer to a Command, and the pipe location storage.
 * Loops through all the parsed commands until it finds a pipe
 * Saves the loop control variable as it's location and modifys pipelocation
 * returns TRUE if there is pipe FALSE if there wasn't one.
 */
int checkForPipe(commandStruct* created_command, int* pipelocation) {
    int loopcontrol = 1;

    for (loopcontrol; loopcontrol < created_command -> argc; loopcontrol++) {

        if (*created_command -> argv[loopcontrol] == '|') {
            *pipelocation = loopcontrol;
            return TRUE;
        }
    }
    return FALSE;
}
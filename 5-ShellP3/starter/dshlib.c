#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"
#include <dirent.h>
#include <sys/utsname.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */
int exec_local_cmd_loop()
{
    char *cmd_buff;
    int rc = 0;
    char blankString[20] = "";
    //command_list_t clist = {0};
    command_list_t *clist = malloc(sizeof(command_list_t));
    cmd_buff_t *cmd = malloc(sizeof(cmd_buff_t));
    cmd_buff = malloc(2*sizeof(char)*SH_CMD_MAX);
    int runPipeline = 0;
    int moveRedirect = 1;

    char** argList = malloc(sizeof(argList) * 20);
    for (int k=0; k < 20; k++){argList[k] = malloc(sizeof(char) * 50);}

    while (1)
    {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL)
        {
            printf("\n");
            break;
        }
        // remove the trailing \n from cmd_buff
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        //rc = build_cmd_list(cmd_buff, &clist);
        rc = build_cmd_list(cmd_buff, clist, cmd);

           if (rc == OK)
           {
                //printf(CMD_OK_HEADER, clist->num + 1);
                //loop through the commands and format the args; set flag as -1 if no args
                for (int commandCount = 0; commandCount <= clist->num; commandCount++)
               {
                    if (strcmp(clist->commands[commandCount]._cmd_buffer, EXIT_CMD) == 0){return 0;}
                    else if (strcmp(clist->commands[commandCount]._cmd_buffer, "dragon") == 0){printDragon();}
                    else if (strcmp(clist->commands[commandCount]._cmd_buffer, "cd") == 0){rc = changeDir(cmd->argv);}

                    else if (strcmp(clist->commands[commandCount].argv, blankString) == 0)
                    {
                        //if the command is marked with the > operator move it to the next one
                        // this is because the build command list function puts this flag on the left command rather than right
                        // ex. cmd1 > file.txt; the > symbol is associated with cmd1.
                        //if (clist->commands[commandCount].argc == 10 && moveRedirect == 1)
                        //{
                        //    clist->commands[commandCount + 1].argc = 10;
                        //    moveRedirect = 0;
                        //    clist->commands[commandCount].argc = 1;
                        //}
                        clist->commands[commandCount].argc *= -1;
                        runPipeline = 1;
                    }
                    
                    else if (strcmp(clist->commands[commandCount].argv, blankString) != 0)
                    {
                        //if the command is marked with the > operator move it to the next one
                        // this is because the build command list function puts this flag on the left command rather than right
                        // ex. cmd1 > file.txt; the > symbol is associated with cmd1.
                        //if (clist->commands[commandCount].argc == 10 && moveRedirect == 1)
                        //{
                        //    clist->commands[commandCount + 1].argc = 10;
                        //    moveRedirect = 0;
                        //    clist->commands[commandCount].argc = 1;
                        //}
                        getArgs(cmd, clist, commandCount, argList);
                        runPipeline = 1;
                    }
                    else printf("\n");

                    //rc = forkExec(clist, commandCount);
                    //printError(rc);
                }

                if (runPipeline == 1)
                {
                    //run fork pipline
                    pid_t supervisor = fork();
                    if (supervisor == -1) {
                        perror("fork supervisor");
                        exit(EXIT_FAILURE);
                    }

                    if (supervisor == 0) {  // Supervisor process
                        execute_pipeline(clist);
                        exit(EXIT_SUCCESS);
                    }

                    // Main parent just waits for supervisor
                    waitpid(supervisor, NULL, 0);
                }
                

                
            }

            if (rc == WARN_NO_CMDS){
                printf(CMD_WARN_NO_CMD);
            }

            if (rc == ERR_TOO_MANY_COMMANDS){
                printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            }
        

        //clear clist
        for (int commandCount = 0; commandCount <= clist->num; commandCount++)
        {
            strcpy(clist->commands[commandCount]._cmd_buffer, "                   ");
            strcpy(clist->commands[commandCount].argv, "                   ");
        }
    }
    //free memory
    free(cmd_buff);
    free(cmd);
    for (int k=0; k < 20; k++){free(argList[k]);}
    free(argList);
    free(clist);
    
}

int build_cmd_list(char *cmd_line, command_list_t *clist, cmd_buff_t *cmd)
{
    
    //return if no input
    if (cmd_line == NULL){return WARN_NO_CMDS;}
    if (strcmp(cmd_line, "") == 0){return WARN_NO_CMDS;}
    if (cmd_line == 0){return WARN_NO_CMDS;}

    int i = 0;
    int j = 0;
    char arg[20] = "                    ";
    int argsFound = 0;
    int argStart = 0;
    int redirectFlag = 0;

    command_t* command = malloc(sizeof(command_t));
    
    //Add a pipe to the end of comd_line so my search works
    strcat(cmd_line, PIPE_STRING);

    //search for "args" in cmd_line and set flag if found
    int rc = searchString(cmd_line,"args");
    if (rc > 0){argsFound = 1;}

    int count = 0;
    while (i < strlen(cmd_line))
    {
        //check if number of commands is greater than 8
        if (count >= 8)
        {
            free(command);
            return ERR_TOO_MANY_COMMANDS;
        }
        char* commandUnparsed = malloc(10*sizeof(char)*strlen(cmd_line));

        //make sure command unparsed is actually empty
        for (int k = 0; k < sizeof(commandUnparsed); k++){commandUnparsed[k] = NULL;}

        //make sure command.exe is actually empty
        for (int k = 0; k < sizeof(command->exe); k++){command->exe[k] = NULL;}
        
        //parse commands using pipe
        //while (cmd_line[i] != PIPE_CHAR)
        while (cmd_line[i] != PIPE_CHAR && cmd_line[i] != '>')
        {
            commandUnparsed[j] = cmd_line[i];
            i++; // i tracks cmd_line position
            j++;// j tracks commandUnparsed position
        }

        // set redirect flag if cmd_line[i] = '>'
        if (cmd_line[i] == '>'){redirectFlag = 1;}
        if (cmd_line[i] == '>' && cmd_line[i+1] == '>')
        {
            i += 2;
            redirectFlag = 2;
        }

        //add a space at the end of command
        commandUnparsed[j] = SPACE_CHAR;

        //if commandUnparsed begins with a space, jump passed it
        j = 0;
        while (commandUnparsed[j] == SPACE_CHAR){j++;}

        //advance j to next space in command (end of command statement)
        while (commandUnparsed[j] != SPACE_CHAR){j++;}

        //copy command to command structure
        strncpy(command->exe, commandUnparsed, j);

        //remove leading and trailing spaces
        formatString(command->exe);

        //if args was found tack it on to the command
        if (argsFound == 1)
        {
            strcat(command->exe, " args");
        }

        //advance j to first argument
        if (argsFound == 0)
        {
            while (commandUnparsed[j] == SPACE_CHAR){j++;}
            argStart = j;
        }
        else argStart = rc +4; // argStart set to the end of where args is found in the input

        //find the end of the argument list (end of unparsed input)
        while (j <= strlen(commandUnparsed)){j++;}

        //clear arg
        for (int argClear = 0; argClear < 20; argClear++){arg[argClear] = NULL;}

        // copy arguments from unparsed buffer to arg container
        int argCount = 0;
        for (int k = argStart; k < j - 1; k++)
        {
            arg[argCount] = commandUnparsed[k];
            argCount++;
        }
        //copy args to command structure
        strcpy(command->args, arg);

        //remove preceding or trailing spaces
        formatString(command->args);

        //add to command structure to clist
        if (redirectFlag == 1){
            cmd->argc = 10;
            redirectFlag = 0;
        }
        else if (redirectFlag == 2)
        {
            cmd->argc = 20;
            redirectFlag = 0;
        }
        else {cmd->argc = 1;}
        //else {cmd->argc = count;}

        strcpy(cmd->_cmd_buffer, command->exe);
        strcpy(cmd->argv, command->args);
        clist->num = count;
        //clist->commands[count] = *command;
        clist->commands[count] = *cmd;

        //keeping searching through input
        i+=1;
        //reset the unparsed buffer pointer
        j=0;
        //add to command count
        count++;
        free(commandUnparsed);

    }

    free(command);
    return OK;
}

int searchString(char *cmd_line, char *searchTerm)
{
    int searchCount = 0;
    int found = 0;
    int foundAt = 0;

    for (int i = 0; i < strlen(cmd_line); i++)
    {
        //look for first letter of searchTerm
        if (cmd_line[i] == searchTerm[searchCount])
        {
            //search for remainder of word
            for (int j = 0; j <= strlen(searchTerm); j++)
            {
                foundAt = i;
                //tracks the total number of letter is the search term found
                if (cmd_line[i + j] == searchTerm[searchCount]){found++;}
                searchCount++;
                if (searchCount == strlen(searchTerm)){break;}
            }
        }
    }
    //if the whole search term is found return the starting index
    if (found == strlen(searchTerm)){return foundAt;}
    return -1;
}

void formatString(char *string)
{
    //                 remove leading zeros
    int count = 0;
    char* temp = malloc(2*sizeof(char)*strlen(string));

    //copy string to temp
    strcpy(temp, string);

    //count forward from the start of the string until you hit real data
    while (string[count] == SPACE_CHAR){count++;}

    //copy temp to string
    for (int i = 0; i < strlen(temp); i++)
    {
        string[i] = temp[count];
        count++;
    }

    free(temp);

    //               remove trailing zeros

    //move index to the end of the string
    count = strlen(string) - 1;

    //count backward from the end of the string until you hit real data
    while (string[count] == 32 || string[count] == 4){count--;}
    temp = malloc(2*sizeof(char)*strlen(string));

    //copy string to temp
    strcpy(temp, string);

    //clear and resize string
    strcpy(string, "");
    for (int i = 0; i < count + 1; i++)
    {
        strcat(string, " ");
    }

    //rewrite string from temp
    for (int i = 0; i < count + 1; i++){string[i] = temp[i];}

    free(temp);
}

//This is for use with anything in quotes
// it clears every up to and including the first quote
void stripLeadingZeros(char *string)
{
    //                 remove leading zeros
    int count = 0;
    char* temp = malloc(2*sizeof(char)*strlen(string));

    //copy string to temp
    strcpy(temp, string);

    //count forward from the start of the string until you hit a quote
    while (string[count] != 34){count++;}

    //copy temp to string
    for (int i = 0; i < strlen(temp); i++)
    {
        string[i] = temp[count + 1];
        count++;
    }

    free(temp);
}

void printWorkingDir()
{
    char cwd[100];
    getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd);
}

int changeDir(char* newDirectory)
{
    char cwd[100];
    getcwd(cwd, sizeof(cwd));
    strcat(cwd, "/");
    strcat(cwd, newDirectory);
    if (chdir(newDirectory) == 0){return 0;}
    return -1;
}

//parses multple args inlcuding anything in quotes
int parseArgs(char** argList, cmd_buff_t *cmd)
{
    char* argCpy = malloc(sizeof(char) * 50);
    char* temp = malloc(sizeof(char) * 50);
    
    int argCount = 0;
    int i=0;
    int argCpyLength;
    
    //append a space to the end so search works
    //strcat(cmd->argv, " ");
    
    //copy cmd->argv to argCpy
    strcpy(argCpy, cmd->argv);
    formatString(argCpy);

    strcat(argCpy, " ");
    //printf("strlen argCpy: %d\n", strlen(argCpy));

    argCpyLength = strlen(argCpy);

    //clear cmd->argv
    for (int k=0; k < strlen(cmd->argv); k++){cmd->argv[k] = SPACE_CHAR;}

    //begin parse
    while (argCpyLength > 0)
    {
        //clear cmd->argv
        strcpy(cmd->argv, "                                                  ");
        formatString(cmd->argv);

        //find first space or quote
        i=0;
        while (argCpy[i] != 32 && argCpy[i] != 34){i++;}

        //if the first thing we find is a space capture that arg to argv
        if (argCpy[i] == SPACE_CHAR){
            strncpy(cmd->argv, argCpy, i);
        }
        
        //if the first thing we find is a quote, find the next quote
        else if (argCpy[i] == 34)
        {
            //find next quote
            int k=1;
            while (argCpy[k] != 34 && k < strlen(argCpy)){k++;}
            
            //        copy to temp and format
            //create temp string
            char* copyString = malloc(sizeof(char)*strlen(argCpy));
            //clear temp string
            strcpy(copyString, " ");
            //move contents of argCpy to temp string
            for (int copy = 0; copy < k; copy++){copyString[copy] = argCpy[copy];}
            for (int copy = k; copy < strlen(copyString); copy++){copyString[copy] = SPACE_CHAR;}
            
            //format temp string
            stripLeadingZeros(copyString);
            //move temp to argv
            strcpy(cmd->argv, copyString);

            i = k+1;
            free(copyString);
        }
        
        //move arg to arg-list
        strcpy(argList[argCount], cmd->argv);
        
        //delete found arg from temp
        strcpy(temp, argCpy);
        for (int k = i+1; k <= 50; k++){argCpy[k-i-1] = temp[k];}
        argCpyLength = strlen(argCpy);
        
        argCount++;
        
    }

    cmd->argc = argCount;

    //free mallocs
    free(temp);
    free(argCpy);
    
    return 0;
}

int forkExec(command_list_t *clist, int commandCount)
{
    if (clist->commands[commandCount].argc == -1)
    {
        char *args[] = {clist->commands[commandCount]._cmd_buffer, NULL, NULL};
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            // Run child process
            execvp(args[0], args);

            // if execvp fails
            exit(errno);
            
        } else {
            // Parent process
            int status;
            errno = 0;
            waitpid(pid, &status, 0); // Wait for the child process to finish
            
            errno = WEXITSTATUS(status);
            return errno;
        }
        
        return 0;
    }
    else
    {
        char *args[] = {clist->commands[commandCount]._cmd_buffer, clist->commands[commandCount].argv, NULL};
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            exit(1);
        } else if (pid == 0) {
            // Run child process
            execvp(args[0], args);

            // if execvp fails
            exit(errno);
            
        } else {
            // Parent process
            int status;
            errno = 0;
            waitpid(pid, &status, 0); // Wait for the child process to finish
            
            errno = WEXITSTATUS(status);
            return errno;
        }
     
        return 0;
    }
}

void getArgs(cmd_buff_t *cmd, command_list_t *clist, int commandCount, char** argList)
{
    strcpy(cmd->argv, clist->commands[commandCount].argv);
    strcpy(cmd->_cmd_buffer, clist->commands[commandCount]._cmd_buffer);
    int rc = parseArgs(argList, cmd);
    strcpy(clist->commands[commandCount].argv, argList[0]);
}

void printError(int rc)
{
    if (rc == 1){printf("No such file or directory\n");}
    if (rc == 2){printf("Command not found in PATH\n");}
    if (rc == 5){printf("I/O error\n");}
    if (rc == 13){printf("Permission Denied\n");}
    if (rc == 20){printf("Not a Directory\n");}
    if (rc == 36){printf("File name too long\n");}
}

void printDragon(void)
{
    //stores compressed dragon image
    char* dragon = " 72@01%04N00 69%06N00 68%06N00 65%01 01%07 11@01N00 64%10 08%07N00"
    " 39%07 02%04@01 09%12@01 04%06 02@01%04N00 34%22 06%28N00 32%26 03%12 01%15N00 31%29 01%19 05%03N00"
    " 29%28@01 01@01%18 08%02N00 28%33 01%22N00 28%58N00 28%50@01%06@01N00 06%08@01 11%16 08%26 06%02N00"
    " 04%13 09%02@01%12 11%11 01%12 06@01%01N00 02%10 03%03 08%14 12%24N00"
    " 01%09 07%01 09%13 13%12@01%11N00%09@01 16%01 01%13 12@01%25N00%08@01 17%02@01%12 12@01%28N00"
    "%07@01 19%15 11%33N00%10 18%15 10%35 06%04N00%09@01 19@01%14 09%12@01 01%04 01%17 03%08N00"
    "%10 18%17 08%13 06%18 01%09N00%09@01%02@01 16%16@01 07%14 05%24 02%02N00"
    " 01%10 18%01 01%14@01 08%14 03%26 01%02N00 02%12 02@01 11%18 08%40 02%03N00"
    " 03%13 01%02 02%01 02%01@01 01%18 10%37 04%03N00 04%18 01%22 11@01%31 04%07N00 05%39 14%28 07%03N00"
    " 06@01%35 18%25N00 08%32 22%19 02%07N00 11%26 27%15 02@01%09N00"
    " 14%20 11@01%01@01%01 18@01%18 03%03N00 18%15 08%10 20%15 04%01N00 16%36 22%14N00 16%26 02%04 01%03"
    " 22%10 02%03@01N00 21%19 01%06 01%02 26%13@01N00 81%07@01N00";

    //print characters from string (character to print is followed by the number of characters in two-digit format)
    for (int i = 0; i < strlen(dragon); i+=3)
    {
        if (dragon[i] == SPACE_CHAR)
        {
            int count = (dragon[i+1] - 48) * 10 + (dragon[i+2] - 48);
            for (int j = 0; j < count; j++){printf(" ");}
        }
        else if (dragon[i] == '@')
        {
        int count = (dragon[i+1] - 48) * 10 + (dragon[i+2] - 48);
        for (int j = 0; j < count; j++){printf("@");}
        }
        else if (dragon[i] == '%')
        {
        int count = (dragon[i+1] - 48) * 10 + (dragon[i+2] - 48);
        for (int j = 0; j < count; j++){printf("%%");}
        }
        else if (dragon[i] == 'N'){printf("\n");}
        }


}

int execute_pipeline(command_list_t *clist) 
{
    int num_commands = clist->num + 1;
    // if > character is found reduce the number of commands by one (so it doesn't try to execute the filename)
    if (clist->commands[0].argc <= -10 || clist->commands[0].argc >= 10){num_commands = clist->num;}
    
    int pipes[num_commands - 1][2];  // Array of pipes
    pid_t pids[num_commands];        // Array to store process IDs

    // Create all necessary pipes
    for (int i = 0; i < num_commands - 1; i++) 
    {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Create processes for each command
    for (int i = 0; i < num_commands; i++) 
    {
        pids[i] = fork();
        if (pids[i] == -1) 
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        
        if (pids[i] == 0) 
        {  // Child process
            //if the redirect indicator is present create a file with the command name
            if (clist->commands[i].argc <= -10 || clist->commands[i].argc >= 10)
            {
                //printf("argc%d\n", clist->commands[i].argc);
                char* filename = clist->commands[i + 1]._cmd_buffer;
                //printf("cmd buffer[%d]: %s\n", i, clist->commands[i]._cmd_buffer);
                //printf("filename: %s\n", filename);
                int out = 0;
                if (clist->commands[i].argc == 20 || clist->commands[i].argc == -20)
                {
                    out = open(filename, O_RDWR|O_CREAT|O_APPEND, 0600);
                }
                else if (clist->commands[i].argc == 10 || clist->commands[i].argc == -10)
                {
                    out = open(filename, O_RDWR|O_CREAT|O_TRUNC, 0600);
                }
                
                if (-1 == out) 
                {
                    perror("opening:"); 
                    printf("%s\n", filename);
                    return 255; 
                }
        
                
                int save_out = dup(fileno(stdout));
                if (-1 == dup2(out, fileno(stdout))) { perror("cannot redirect stdout"); return 255; }

                // Execute command if no arguments
                if (clist->commands[i].argc < 0)
                {
                    char *args[] = {clist->commands[i]._cmd_buffer, NULL, NULL};
                    execvp(args[0], args);
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
                
                // Execute command with arguments
                else if (clist->commands[i].argc > 0)
                {
                    char *args[] = {clist->commands[i]._cmd_buffer, clist->commands[i].argv, NULL};
                    execvp(args[0], args);
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }

                fflush(stdout); close(out);

                dup2(save_out, fileno(stdout));

                close(save_out);

            }

            // if not a redirect create pipes as usual
            if (clist->commands[i].argc == -1 || clist->commands[i].argc == 1)
            {
                // Set up input pipe for all except first process
                if (i > 0) {dup2(pipes[i-1][0], STDIN_FILENO);}
                
                // Set up output pipe for all except last process
                if (i < num_commands - 1) {dup2(pipes[i][1], STDOUT_FILENO);}

                // Close all pipe ends in child
                for (int j = 0; j < num_commands - 1; j++) 
                {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                
                // Execute command if no arguments
                if (clist->commands[i].argc < 0)
                {
                    char *args[] = {clist->commands[i]._cmd_buffer, NULL, NULL};
                    execvp(args[0], args);
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
                
                // Execute command with arguments
                else if (clist->commands[i].argc > 0)
                {
                    char *args[] = {clist->commands[i]._cmd_buffer, clist->commands[i].argv, NULL};
                    execvp(args[0], args);
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
            
        }
    }

    // Parent process: close all pipe ends
    for (int i = 0; i < num_commands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all children
    for (int i = 0; i < num_commands; i++) {
        waitpid(pids[i], NULL, 0);
    }
}
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
    extern void print_dragon();
    //char *cmd_buff;
    int rc = 0;
    cmd_buff_t *cmd = malloc(sizeof(cmd_buff_t));

    // TODO IMPLEMENT MAIN LOOP
    char* cmd_buff = malloc(ARG_MAX * sizeof(char));
    
    // initialize argList
    char** argList = malloc(sizeof(argList) * 20);
    for (int k=0; k < 20; k++){argList[k] = malloc(sizeof(char) * 50);}

    while (1)
    {
        //clear cmd structure
        strcpy(cmd->_cmd_buffer, "                                       ");
        strcpy(cmd->argv, "                                       ");
        cmd->argc = 0;
        // clear argList
        for (int k=0; k < 20; k++){strcpy(argList[k], " ");}

        //get input
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL)
        {
            printf("\n");
            break;
        }
        // remove the trailing \n from cmd_buff
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

    // TODO IMPLEMENT parsing input to cmd_buff_t *cmd_buff
        rc = parse_cmd_buff(cmd_buff, cmd);
        
        
    // TODO IMPLEMENT if built-in command, execute builtin logic for exit, cd (extra credit: dragon)
    // the cd command should chdir to the provided directory; if no directory is provided, do nothing
    // TODO if exit command, exit return code 0
    if (strcmp(cmd->_cmd_buffer, EXIT_CMD) == 0)
    {
        free(cmd_buff);
        free(cmd);
        for (int k=0; k < 20; k++){free(argList[k]);}
        free(argList);
        return 0;
    }

    if (strcmp(cmd->_cmd_buffer, "dragon") == 0){void call_function();{print_dragon();}}

    if (strcmp(cmd->_cmd_buffer, "pwd") == 0){printWorkingDir();}

    if (strcmp(cmd->_cmd_buffer, "cd") == 0){rc = changeDir(cmd->argv);}

    if (strcmp(cmd->_cmd_buffer, "echo") == 0)
    {
        int rc = parseArgs(argList, cmd);
        //print args
        for (int j=0; j < cmd->argc; j++){printf("%s\n", argList[j]);}
        
        if (rc == -1)
        {
            free(cmd_buff);
            free(cmd);
            for (int k=0; k < 20; k++){free(argList[k]);}
            free(argList);
            return -1;
        }
    }
    
    // TODO IMPLEMENT if not built-in command, fork/exec as an external command
    // for example, if the user input is "ls -l", you would fork/exec the command "ls" with the arg "-l"
    if (strcmp(cmd->_cmd_buffer, "uname") == 0)
    {
        struct utsname buffer;
        errno = 0;
        if (uname(&buffer) < 0)
        {
            perror("uname");
            exit(EXIT_FAILURE);
        }
            int rc = parseArgs(argList, cmd);
            if (strcmp(argList[0], "-a") == 32)
            {
                char* arg = "-a";
                rc = forkExec(cmd, arg);
                //printf("%s ", buffer.version);
                //printf("%s ", buffer.release);
                //printf("%s ", buffer.sysname);
                //printf("%s ", buffer.machine);
                //printf("%s\n", buffer.nodename);
            }
            else
             {
                char* arg = NULL;
                rc = forkExec(cmd, arg);
                //printf("%s\n", buffer.sysname);
             }

            #ifdef _GNU_SOURCE
             printf("domain name = %s\n", buffer.domainname);
            #endif
        
    }

    if (strcmp(cmd->_cmd_buffer, "ls") == 0)
    {
        int rc = parseArgs(argList, cmd);
        if (strcmp(argList[0], "-l") == 32)
        {
            char* arg = "-l";
            rc = forkExec(cmd, arg);   
            //rc = listDirDetailed();
        }
        else 
        {
            char* arg = NULL;
            rc = forkExec(cmd, arg);
        };

        if (rc == -1)
        {
            free(cmd_buff);
            free(cmd);
            for (int k=0; k < 20; k++){free(argList[k]);}
            free(argList);
            return -1;
        }
    }

    if (strcmp(cmd->_cmd_buffer, "which") == 0)
    {
        int rc = parseArgs(argList, cmd);
        if (strcmp(argList[0], "which") == 32)
        {
            char* arg = "which";
            rc = forkExec(cmd, arg);   
            //rc = listDirDetailed();
        }
        else 
        {
            char* arg = NULL;
            rc = forkExec(cmd, arg);
        };

        if (rc == -1)
        {
            free(cmd_buff);
            free(cmd);
            for (int k=0; k < 20; k++){free(argList[k]);}
            free(argList);
            return -1;
        }
    }


    }

    //free memory
    free(cmd_buff);
    free(cmd);
    for (int k=0; k < 20; k++){free(argList[k]);}
    free(argList);
    return OK;
}

int parse_cmd_buff(char *cmd_buff, cmd_buff_t *cmd)
{
    //return if no input
    if (cmd_buff == NULL){return WARN_NO_CMDS;}
    if (strcmp(cmd_buff, "") == 0){return WARN_NO_CMDS;}
    if (cmd_buff == 0){return WARN_NO_CMDS;}

    //printf("cmd buff: %d\n", cmd_buff[0]);

    int i = 0;
    int j = 0;
    char* arg = malloc (sizeof(char)*50);
    int argsFound = 0;
    int argStart = 0;

    //Add a pipe to the end of comd_line so my search works
    strcat(cmd_buff, PIPE_STRING);

    //search for "args" in cmd_line and set flag if found
    int rc = searchString(cmd_buff,"args");
    if (rc > 0){argsFound = 1;}
    
    int count = 0;
    while (i < strlen(cmd_buff))
    {
        
        char* commandUnparsed = malloc(sizeof(char)*strlen(cmd_buff));
        
        //make sure command unparsed is actually empty
        for (int k = 0; k < sizeof(commandUnparsed); k++)
        {
            commandUnparsed[k] = NULL;
        }

        //parse commands using pipe
        while (cmd_buff[i] != PIPE_CHAR)
        {
            commandUnparsed[j] = cmd_buff[i];
            i++; // i tracks cmd_line position
            j++;// j tracks commandUnparsed position
        }
        
        //add a space at the end of command
        commandUnparsed[j] = SPACE_CHAR;
        
        //if commandUnparsed begins with a space, jump passed it
        j = 0;
        while (commandUnparsed[j] == SPACE_CHAR){j++;}

        //advance j to next space in command (end of command statement)
        while (commandUnparsed[j] != SPACE_CHAR){j++;}
        
        //copy command to command structure
        strncpy(cmd->_cmd_buffer, commandUnparsed, j);
        
        
        //if args was found tack it on to the command
        formatString(cmd->_cmd_buffer);
        if (argsFound == 1)
        {
            strcat(cmd->_cmd_buffer, " args");
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
        //for (int argClear = 0; argClear < 20; argClear++){arg[argClear] = SPACE_CHAR;}
        for (int argClear = 0; argClear < 20; argClear++){arg[argClear] = NULL;}

        // copy arguments from unparsed buffer to arg container
        int argCount = 0;
        for (int k = argStart; k < j - 1; k++)
        {
            arg[argCount] = commandUnparsed[k];
            argCount++;
        }
        
        //copy args to command structure
        strcpy(cmd->argv, arg);
        
        //remove preceding or trailing spaces
        formatString(cmd->argv);

        //keeping searching through input
        i+=1;
        //reset the unparsed buffer pointer
        j=0;
        //add to command count
        count++;
        
        free(commandUnparsed);
        free(arg);
        
    }

    //free(command);
    return OK;
    return 0;
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

//not used - prints directory without for-exec
int listDir(void)
{
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char date_time[20];

    dir = opendir(".");
    if (dir == NULL){return -1;}
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")){printf("%s  ", entry->d_name);}
    }
    printf("\n");
    return 0;
}

//not used - prints directory without for-exec (same as -l)
int listDirDetailed(void)
{
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char date_time[20];

    dir = opendir(".");
    if (dir == NULL){return -1;}
    while ((entry = readdir(dir)) != NULL)
    {
        if (stat(entry->d_name, &file_stat) == 0) {
            strftime(date_time, sizeof(date_time), "%Y-%m-%d %H:%M:%S", localtime(&file_stat.st_mtime));
            printf("%s\t%s\n", date_time, entry->d_name);
        } else 
        {
            perror("Error getting file stats");
        }
    }
    printf("\n");
    return 0;
}

//parses multple args inlcuding anything in quotes
int parseArgs(char** argList, cmd_buff_t *cmd)
{
    char* argCpy = malloc(sizeof(char) * 50);
    char* temp = malloc(sizeof(char) * 50);
    
    int argCount = 0;
    int i=0;
    
    
    //append a space to the end so search works
    strcat(cmd->argv, " ");
    
    //copy cmd->argv to argCpy
    strcpy(argCpy, cmd->argv);

    //clear cmd->argv
    for (int k=0; k < strlen(cmd->argv); k++){cmd->argv[k] = SPACE_CHAR;}
    //strcpy(cmd->argv, "");

    //begin parse
    while (strlen(argCpy) > 0)
    {
        //clear cmd->argv
        strcpy(cmd->argv, "                                                  ");
        
        //find first space or quote
        i=0;
        while (argCpy[i] != 32 && argCpy[i] != 34){i++;}

        //if the first thing we find is a space capture that arg to argv
        if (argCpy[i] == SPACE_CHAR){strncpy(cmd->argv, argCpy, i);}
        
        //if the first thing we find is a quote, find the next quote
        else if (argCpy[i] == 34)
        {
            //find next quote
            int k=1;
            while (argCpy[k] != 34 && k < strlen(argCpy)){k++;}
            
            //copy to temp and format
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
        
        argCount++;
        
    }

    cmd->argc = argCount;

    //free mallocs
    free(temp);
    free(argCpy);
    
    return 0;
}

int forkExec(cmd_buff_t *cmd, char* arg)
{
    
    char *args[] = {cmd->_cmd_buffer, arg, NULL};
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(1);
    } else if (pid == 0) {
        // Run child process
        execvp(args[0], args);
        
        // if execvp fails
        perror("execvp"); 
        exit(1);
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0); // Wait for the child process to finish
        //printf("Child process finished with status %d\n", WEXITSTATUS(status));
    }
     
    return 0;
}
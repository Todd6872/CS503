#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */
int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    if (strcmp(cmd_line, "") == 0){return WARN_NO_CMDS;}
    int i = 0;
    int j = 0;
    char arg[20] = "                    ";
    int argsFound = 0;
    int argStart = 0;
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
        for (int k = 0; k < sizeof(commandUnparsed); k++)
        {
            commandUnparsed[k] = NULL;
        }
        //make sure command.exe is actually empty
        for (int k = 0; k < sizeof(command->exe); k++)
        {
            command->exe[k] = NULL;
        }

        //parse commands using pipe
        while (cmd_line[i] != PIPE_CHAR)
        {
            commandUnparsed[j] = cmd_line[i];
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
        strncpy(command->exe, commandUnparsed, j);

        //if args was found tack it on to the command
        formatString(command->exe);
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
        for (int argClear = 0; argClear < 20; argClear++){arg[argClear] = SPACE_CHAR;}

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
        clist->num = count;
        clist->commands[count] = *command;

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

void printDragon(void)
{

#define line1 "                                                                        @%%%%%%%%\n"
#define line2 "                                                                     %%%%%%%%%%%%\n"
#define line3 "                                                                    %%%%%%%%%%%%\n"
#define line4 "                                                                 %% %%%%%%%%%%%%%%           @\n"
#define line5 "                                                                %%%%%%%%%%%%%%%%%%%%        %%%%%%%%%%%%%%\n"
#define line6 "                                       %%%%%%%%%%%%%%  %%%%%%%%@         %%%%%%%%%%%%%%%%%%%%%%%%@    %%%%%%%%%%%%  @%%%%%%%%\n"
#define line7 "                                  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%      %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n"
#define line8 "                                %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%   %%%%%%%%%%%%%%%%%%%%%%%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n"
#define line9 "                               %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%     %%%%%%\n"
#define line10 "                             %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%@ @%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%         %%%%\n"
#define line11 "                            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n"
#define line12 "                            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n"
#define line13 "                            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%@%%%%%%%%%%%%@\n"
#define line14 "      %%%%%%%%%%%%%%%%@           %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%      %%%%\n"
#define line15 "    %%%%%%%%%%%%%%%%%%%%%%%%%%         %%%%@%%%%%%%%%%%%%%%%%%%%%%%%           %%%%%%%%%%%%%%%%%%%%%% %%%%%%%%%%%%%%%%%%%%%%%%      @%%\n"
#define line16 "  %%%%%%%%%%%%%%%%%%%%   %%%%%%        %%%%%%%%%%%%%%%%%%%%%%%%%%%%            %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n"
#define line17 " %%%%%%%%%%%%%%%%%%        %%         %%%%%%%%%%%%%%%%%%%%%%%%%%              %%%%%%%%%%%%%%%%%%%%%%%%@%%%%%%%%%%%%%%%%%%%%%%\n"
#define line18 "%%%%%%%%%%%%%%%%%%@                %% %%%%%%%%%%%%%%%%%%%%%%%%%%            @%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n"
#define line19 "%%%%%%%%%%%%%%%%@                 %%%%@%%%%%%%%%%%%%%%%%%%%%%%%            @%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n"
#define line20 "%%%%%%%%%%%%%%@                   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%           %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n"
#define line21 "%%%%%%%%%%%%%%%%%%%%                  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%          %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%      %%%%%%%%\n"
#define line22 "%%%%%%%%%%%%%%%%%%@                   @%%%%%%%%%%%%%%%%%%%%%%%%%%%%         %%%%%%%%%%%%%%%%%%%%%%%%@ %%%%%%%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%   %%%%%%%%%%%%%%%%\n"
#define line23 "%%%%%%%%%%%%%%%%%%%%                  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%        %%%%%%%%%%%%%%%%%%%%%%%%%%      %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%%%%%%%%%%%%%%%%%\n"
#define line24 "%%%%%%%%%%%%%%%%%%@%%%%@                %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%@       %%%%%%%%%%%%%%%%%%%%%%%%%%%%     %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  %%%%\n"
#define line25 " %%%%%%%%%%%%%%%%%%%%                  %% %%%%%%%%%%%%%%%%%%%%%%%%%%%%@        %%%%%%%%%%%%%%%%%%%%%%%%%%%%   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%%%\n"
#define line26 "  %%%%%%%%%%%%%%%%%%%%%%%%  @           %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  %%%%%%\n"
#define line27 "   %%%%%%%%%%%%%%%%%%%%%%%%%% %%%%  %%  %%@ %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%          %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    %%%%%%\n"
#define line28 "    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%           @%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    %%%%%%%%%%%%%%\n"
#define line29 "     %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%              %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%       %%%%%%\n"
#define line30 "      @%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%                  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n"
#define line31 "        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%                      %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  %%%%%%%%%%%%%%\n"
#define line32 "           %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%                           %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  @%%%%%%%%%%%%%%%%%%\n"
#define line33 "              %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%           @%%@%%                  @%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%   %%%%%%\n"
#define line34 "                  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%        %%%%%%%%%%%%%%%%%%%%                    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    %%\n"
#define line35 "                %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%                      %%%%%%%%%%%%%%%%%%%%%%%%%%%%\n"
#define line36 "                %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  %%%%%%%% %%%%%%                      %%%%%%%%%%%%%%%%%%%%  %%%%%%@\n"
#define line37 "                     %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% %%%%%%%%%%%% %%%%                          %%%%%%%%%%%%%%%%%%%%%%%%%%@\n"
#define line38 "                                                                                 %%%%%%%%%%%%%%@\n"


    printf( line1);
    printf( line2);
    printf( line3);
    printf( line4);
    printf( line5);
    printf( line6);
    printf( line7);
    printf( line8);
    printf( line9);
    printf( line10);
    printf( line11);
    printf( line12);
    printf( line13);
    printf( line14);
    printf( line15);
    printf( line16);
    printf( line17);
    printf( line18);
    printf( line19);
    printf( line20);
    printf( line21);
    printf( line22);
    printf( line23);
    printf( line24);
    printf( line25);
    printf( line26);
    printf( line27);
    printf( line28);
    printf( line29);
    printf( line30);
    printf( line31);
    printf( line32);
    printf( line33);
    printf( line34);
    printf( line35);
    printf( line36);
    printf( line37);
    printf( line38);
}
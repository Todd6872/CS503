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
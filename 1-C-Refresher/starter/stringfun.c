#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50
#define STRING_SZ 70

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);
//add additional prototypes here
int reverseString(char *, int, int);
int wordPrint(char *, int, int);
int customStrLength(char *);
int searchReplace(char *, char*, char*);
void deleteChar(char *, int);
void addChar(char *, int, char);
int findWord(char *, char*);

int setup_buff(char *buff, char *user_str, int len){
    // Implement the setup buff as per the directions
    int iBuffer = 0; //buffer pointer
    int iUser = 0; //input string pointer
    char buffCurrent = '0'; //hold contents of the buffer at location iBuffer
    int spaceCount = 0; //counts the number of sequential spaces encountered

    while (*(user_str + iUser) == 32 || *(user_str + iUser) == 9){iUser++;} // jump the string pointer over any leading spaces

    //copy contents of input string over to buffer
    while (buffCurrent != '\0'){
        *(buff + iBuffer) = *(user_str + iUser); //move contents of user_str to buff using pointers
        //printf("buffer contents:%c,%d\n", *(buff + iBuffer), *(buff + iBuffer));
        if (*(buff + iBuffer) == 9){*(buff +iBuffer) = 32;} //if there's a tab make it a space instead
        buffCurrent = *(buff + iBuffer); //stores current buffer contents
        if (buffCurrent == 32){spaceCount++;}
        iBuffer++;
        iUser++;
        if (spaceCount > 0){
            while (*(user_str + iUser) == 9 || *(user_str + iUser) == 32){iUser++;}
            spaceCount = 0;
        }

    }

    //replace any spaces on back end with dots
    iBuffer = len;
    while (*(buff + iBuffer) == 32 || *(buff + iBuffer) == 46 || *(buff + iBuffer) == 0)
    {
        *(buff + iBuffer) = 46;
        iBuffer--;
    }

    if (iUser - 1 > len){return -1;} //user string too large
    int stringLength = customStrLength (buff); //run custom string length function
    if (stringLength > BUFFER_SZ){return -1;}
    return stringLength; //length of buffer string
}

void print_buff(char *buff, int len){
    printf("Buffer:  [");
    for (int i=0; i<BUFFER_SZ; i++){
        putchar(*(buff+i));
    }
    printf("]\n");
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);

}

int count_words(char *buff, int len, int str_len){
    int spaceCount = 0;
    for (int i=0; i<len; i++)
    {
        if (*(buff + i) == 32){spaceCount++;}
    }
    if (spaceCount > str_len){return -1;}
    if (spaceCount == 0){return -1;}
    return (spaceCount + 1);
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS
int reverseString(char *buff, int str_len, int len)
{
    if (str_len <=0){return -1;}
    char *temp = malloc(str_len);
    int count = 0;

    //move buff contents into temp
    for (int i = 0; i <= str_len - 1; i++)
    {
        *(temp + i) = *(buff + i);
    }

    //write contents of temp into buff in reverse order
    count = str_len - 1;
    for (int i = 0; i <= str_len - 1; i++)
    {
        *(buff + i) = *(temp + count);
        count--;
    }
    free(temp);
    return 0;
}

int wordPrint(char *buff, int str_len, int len)
{
    printf("Word Print\n");
    printf("----------\n");
    int wordLength = 0;
    int wordCount = 1;
    int newWord = 1;

    for (int i=0; i < str_len; i++)
    {
        //if you're at the end of the user string, add the string length verbiage
        if (i >= str_len - 1)
        {

            printf("%c", *(buff + i));
            printf("(%d)\n", wordLength  + 1);
        }
        //if I'm in the middle of a word print the next character. If I'm starting a new word then
        //print word count and reset the new word flag.
        else if (*(buff + i) != 32)
        {
            if (newWord == 1)
            {
                printf("%d. ", wordCount);
                wordCount++;
                newWord = 0;
            }
            printf("%c", *(buff + i));
            wordLength++;
        }
        //if I hit a space print the word length and start a new word
        else if (*(buff + i) == 32)
        {
            printf("(%d)\n", wordLength);
            wordLength = 0;
            newWord = 1;
        }

    }
    printf("\n");
    printf("Number of words returned: %d\n", wordCount - 1);
    if (wordCount == 0){return -1;}
    return wordCount - 1;
}

int customStrLength(char *buff)
{
    int i = 0;
    int count = 0;
    while (*(buff + i) != '\0')
    {
        if (*(buff + i) != 46){count++;}
        i++;
    }
    return count;
}

int searchReplace(char *buff, char* search, char* replace)
{
    int found = 0;
    int searchLength = customStrLength(search);
    int replaceLength = customStrLength(replace);

    //find search term in buff
    found = findWord(buff, search);

    //delete the search string from buff
    for (int i = 0; i < searchLength; i++)
    {
        deleteChar(buff, found);
    }

    //add replace word two buff
    addChar(buff, found, 32);
    for (int i = replaceLength - 1; i >= 0; i--)
    {
        addChar(buff, found, *(replace + i));
    }
    deleteChar(buff, found);

    //correct back end dots
    int counter = BUFFER_SZ;
    while (*(buff + counter) == 46 || *(buff + counter) == 32)
    {
        *(buff + counter) = 46;
        counter--;
    }

    if (found < 1){return -1;}
    return 0;
}

void deleteChar(char* buff, int location)
{
    //int length = customStrLength(buff);
    for (int i = location; i < BUFFER_SZ; i++)
        {
            *(buff + i) = *(buff + i + 1);
        }
}

void addChar(char* buff, int location, char c)
{
    *(buff + location) = c;
    for (int i = BUFFER_SZ; i > location; i--)
    {
       *(buff + i) = *(buff + i - 1);
    }
}

int findWord(char* buff, char* search)
{
    int found = 0;
    int length = customStrLength(buff);
    int searchLength = customStrLength(search);

    for (int i = 0; i < length - 1; i++)
    {
        if (*(buff + i) == *(search + 0)) // search to match the first letter of the search word in buff
        {
            for (int j = 1; j < searchLength; j++) // test to see if the rest of the word matches
            {
                if (*(buff + (i+j)) == *(search + j))
                {
                    found = i;
                }
                else found = -1;
            }
            if (found != -1){break;}
        }
    }
    return found;
}
int main(int argc, char *argv[]){

    char *buff;             //placeholder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string
    char *search_string;    //search word for search and replace
    char *replace_string;   //replace word for search and replace

    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string
    search_string = argv[3]; //search term
    replace_string = argv[4]; //replace term

    buff = malloc (STRING_SZ); //Allocate space for the buffer using malloc and
    if (buff == NULL){exit(99);} //handle error if malloc fails by exiting with a return code of 99

    user_str_len = setup_buff(buff, input_string, STRING_SZ);     //see todos

    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, STRING_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        //Implement the other cases for 'r' and 'w' by extending the case statement options
        case 'r':
            rc = reverseString(buff, user_str_len, STRING_SZ);
            if (rc < 0){
                printf("Error reversing words, rc = %d", rc);
                exit(2);
            }
            break;
        case 'w':
            rc = wordPrint(buff, user_str_len, STRING_SZ);
            if (rc < 0){
                printf("Error printing words, rc = %d", rc);
                exit(2);
            }
        break;
    case 'x':
        rc = searchReplace(buff, search_string, replace_string);
        if (rc < 0){
            printf("Error replacing word, rc = %d", rc);
            exit(2);
        }
        break;
        default:
            usage(argv[0]);
            exit(1);
    }

    print_buff(buff,STRING_SZ);
    free(buff); //Dont forget to free your buffer before exiting
    exit(0);
}

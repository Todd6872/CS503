#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50

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

int setup_buff(char *buff, char *user_str, int len){
    // Implement the setup buff as per the directions
    //fill buff with .'s
    for(int i = 0; i < len; i++)
    {
        *(buff + i) = '.';
    }
    int iBuffer = 0;
    int iUser = 0;
    char buffCurrent = '0';
    int spaceCount = 0;
    while (buffCurrent != '\0'){
        *(buff + iBuffer) = *(user_str + iUser); //move contents of user_str to buff using pointers
        if (*(buff + iBuffer) == 9){*(buff +iBuffer) = 32;}
        buffCurrent = *(buff + iBuffer); //stores current buffer contents
        if (buffCurrent == 32){spaceCount++;}
        iBuffer++;
        iUser++;
        if (spaceCount > 1){
            while (*(user_str + iUser) == 9 || *(user_str + iUser) == 32)
            {
                iUser++;
            }
            spaceCount = 0;
        }
        //printf ("buff contents: %c\n", buffCurrent);

    }
    iBuffer--;
    *(buff + iBuffer) = '.';
    //printf("spacecount: %d\n", spaceCount);
    //printf ("iUser: %d\n", iUser);
    if (iUser - 1 > len){return -1;} //user string too large
    int stringLength = customStrLength (buff);
    return stringLength; //length of buffer string
}

void print_buff(char *buff, int len){
    printf("Buffer:  ");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    putchar('\n');
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
    return (spaceCount + 1);
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS
int reverseString(char *buff, int str_len, int len)
{
    if (str_len <=0){return -1;}
    //char *temp = malloc(str_len);
    int count = 0;
    printf("Reversed String: ");
    for (int i = str_len - 1; i >= 0; i--)
    {
        //*(temp + count) = *(buff + i);

        putchar(*(buff + i));
        count++;
    }
    printf("\n");

    //for (int i = 0; i <= str_len - 1; i++)
    //{
    //    *(buff + i) = *(temp + i);
    //}
    //free(temp);
    return 0;
}

int wordPrint(char *buff, int str_len, int len)
{
    printf("'''\n");
    printf("Word Print\n");
    printf("----------\n");
    int wordLength = 0;
    int wordCount = 1;
    int newWord = 1;
    //printf("string length: %d\n", str_len);
    for (int i=0; i<str_len; i++)
    {
        //printf("i: %d\n", i);
        if (i >= str_len - 1)
        {

            printf("%c", *(buff + i));
            printf(" (%d)\n", wordLength  + 1);
        }
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
        else if (*(buff + i) == 32)
        {
            printf(" (%d)\n", wordLength);
            wordLength = 0;
            newWord = 1;
        }

    }
    //printf("\n");
    printf("'''\n");
    return 0;
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
int main(int argc, char *argv[]){

    char *buff;             //placeholder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

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
    buff = malloc (BUFFER_SZ); //Allocate space for the buffer using malloc and
    //if (*buff == 0){exit(99);} //handle error if malloc fails by exiting with a return code of 99

    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    //printf("str len: %d", user_str_len);
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        //Implement the other cases for 'r' and 'w' by extending the case statement options
        case 'r':
            rc = reverseString(buff, user_str_len, BUFFER_SZ);
            if (rc < 0){
                printf("Error reversing words, rc = %d", rc);
                exit(2);
            }
            break;
        case 'w':
            rc = wordPrint(buff, user_str_len, BUFFER_SZ);
            if (rc < 0){
            printf("Error printing words, rc = %d", rc);
            exit(2);
            }
        break;
        default:
            usage(argv[0]);
            exit(1);
    }

    print_buff(buff,BUFFER_SZ);
    free(buff); //Dont forget to free your buffer before exiting
    exit(0);
}

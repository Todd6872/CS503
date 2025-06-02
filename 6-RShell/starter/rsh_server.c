
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>
#include <pthread.h>
#include "dshlib.h"
#include "rshlib.h"

static int is_threaded_server = false;
pthread_mutex_t lock;

int start_server(char *ifaces, int port, int is_threaded){
    int svr_socket;
    int rc;
    
    //
    //TODO:  If you are implementing the extra credit, please add logic
    //       to keep track of is_threaded to handle this feature
    //
    is_threaded_server = is_threaded;

    svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0){
        int err_code = svr_socket;
        return err_code;
    }
    
    rc = process_cli_requests(svr_socket);
    

    stop_server(svr_socket);


    return rc;
}

int stop_server(int svr_socket){
    return close(svr_socket);
}

int boot_server(char *ifaces, int port){
    int svr_socket;
    int ret;
    
    struct sockaddr_in addr;

    // TODO set up the socket - this is very similar to the demo code
    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket == -1) {
        perror("socket");
        return ERR_RDSH_COMMUNICATION;
    }

    int enable=1;
    setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    /* Bind socket to socket name. */
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ifaces);
    addr.sin_port = htons(port);

    ret = bind(svr_socket, (const struct sockaddr *) &addr,
               sizeof(struct sockaddr_in));
    if (ret == -1) {
        perror("bind");
        return ERR_RDSH_COMMUNICATION;
    }

    ret = listen(svr_socket, 20);
    if (ret == -1) {
        perror("listen");
        return ERR_RDSH_COMMUNICATION;
    }

    return svr_socket;
}

int process_cli_requests(int svr_socket){
    int     cli_socket;
    int     rc = OK;    

    while(1){
        // TODO use the accept syscall to create cli_socket 
        // and then exec_client_requests(cli_socket)
        cli_socket = accept(svr_socket, NULL, NULL);
        if (cli_socket == -1){
            perror("accept");
            return ERR_RDSH_COMMUNICATION;
        }
        
        
        if(is_threaded_server == false)
        {
            // single threaded
            rc = exec_client_requests(cli_socket);
            if (rc < 0){break;}
        } else 
        {
            // multi threaded
            rc = exec_client_thread(svr_socket, cli_socket);
            if (rc < 0){break;}
        }
    }

    stop_server(cli_socket);
    return rc;
}

 //extra credit threaded handler
typedef struct thread_info{
    int server_socket;
    int client_socket;
}thread_info_t;

int exec_client_thread(int main_socket, int cli_socket) {
    thread_info_t *tinfo = malloc(sizeof(thread_info_t));
    pthread_t thread_id;

    tinfo->server_socket = main_socket;
    tinfo->client_socket = cli_socket;

    if (pthread_mutex_init(&lock, NULL) != 0) 
    {
        printf("\n mutex init has failed\n");
        return 1;
    }

    if (pthread_create(&thread_id, NULL, handle_client, (void *)tinfo) < 0) {
        perror("could not create thread");
        close(tinfo->client_socket);
        free(tinfo);
        return OK;
    }
    
    // Detach thread - resources releaased after running
    pthread_detach(thread_id);

    return OK;
}

void *handle_client(void *arg) 
{ 
    
    thread_info_t *tinfo = (thread_info_t *)arg;
    int rc;

    //handles client requests in loop.
    pthread_mutex_lock(&lock);
    rc = exec_client_requests(tinfo->client_socket);
    printf("rc: %d\n", rc);
    pthread_mutex_unlock(&lock);
    if (rc == OK)
    {
        free(tinfo);
        //just return since this is a detached thread handler
        return NULL;
    } else 
    {
        
        //close on error
        close(tinfo->server_socket);
        free(tinfo);
        exit(0);
    }
}

int exec_client_requests(int cli_socket) {
    int io_size;
    int rc;
    int cmd_rc;
    int last_rc;
    char *io_buff;
    cmd_buff_t *cmd = malloc(sizeof(cmd_buff_t));
    command_list_t *clist = malloc(sizeof(command_list_t));
    char** argList = malloc(sizeof(argList) * 20);
    for (int k=0; k < 20; k++){argList[k] = malloc(sizeof(char) * 50);}
    char blankString[20] = "";
    int runPipeline = 1;
    
    io_buff = malloc(RDSH_COMM_BUFF_SZ);
    if (io_buff == NULL){
        return ERR_RDSH_SERVER;
    }
    while(1) {
        
        runPipeline = 1;
        //clear buffers
        memset(io_buff, 0, RDSH_COMM_BUFF_SZ);
        // TODO use recv() syscall to get input
        
        io_size = recv(cli_socket, io_buff, RDSH_COMM_BUFF_SZ, 0);
        if (io_size == -1){
            perror("recv");
            free(io_buff);
            close(cli_socket);
            return ERR_RDSH_COMMUNICATION;
        }
        
        //client terminated gracefully if I receive zero bytes
        if (io_size == 0){
            printf(RCMD_MSG_CLIENT_EXITED);
            //return 0;
            break;      //leave loop, close connection
        }

        
        // TODO build up a cmd_list
        rc = build_cmd_list((char *)io_buff, clist, cmd);
        
        switch (rc) 
        {
            case ERR_MEMORY:
                sprintf((char *)io_buff, CMD_ERR_RDSH_ITRNL, ERR_MEMORY);
                send_message_string(cli_socket, (char *)io_buff);
                continue;
            case WARN_NO_CMDS:
                sprintf((char *)io_buff, CMD_ERR_RDSH_ITRNL, WARN_NO_CMDS);
                //send_message_string(cli_socket, (char *)io_buff);
                continue;
            case ERR_CMD_OR_ARGS_TOO_BIG:
                sprintf((char *)io_buff, CMD_ERR_PIPE_LIMIT, CMD_MAX);
                send_message_string(cli_socket, (char *)io_buff);
                continue;
            default:
                break;
        }

        for (int commandCount = 0; commandCount <= clist->num; commandCount++)
               {
                    if (strcmp(clist->commands[commandCount]._cmd_buffer, "cd") == 0)
                    {
                        changeDir(clist->commands[commandCount].argv);
                        runPipeline = 0;
                    }
                    
                    else if (strcmp(clist->commands[commandCount]._cmd_buffer, "stop-server") == 0)
                    {
                        printf(RCMD_MSG_SVR_STOP_REQ);
                        free(io_buff);
                        close(cli_socket);
                        free(cmd);
                        for (int k=0; k < 20; k++){free(argList[k]);}
                        free(argList);
                        free(clist);
                        return -10;
                    }

                    else if (strcmp(clist->commands[commandCount]._cmd_buffer, "exit") == 0)
                    {
                        printf(RCMD_MSG_CLIENT_EXITED);
                        return 0;
                    }

                    //if the command has no args
                    else if (strcmp(clist->commands[commandCount].argv, blankString) == 0)
                    {
                        //set no-argument flag for pipe function
                        clist->commands[commandCount].argc *= -1;
                    }
                    
                    //if the command has args
                    else if (strcmp(clist->commands[commandCount].argv, blankString) != 0)
                    {
                        getArgs(cmd, clist, commandCount, argList);
                    }
                    else 
                    runPipeline = 1;
                    printf("\n");

                }
        
        
        
        // TODO rsh_execute_pipeline to run your cmd_list
        last_rc = cmd_rc;
        
        if (runPipeline){cmd_rc = rsh_execute_pipeline(cli_socket, clist);}
        
        //printf("cmd rc: %d\n", cmd_rc);
        
        // TODO send appropriate respones with send_message_string
        // - error constants for failures
        // - buffer contents from execute commands
        //  - etc.
        switch(cmd_rc){
            case RC_SC:
                sprintf((char *)io_buff, RCMD_MSG_SVR_RC_CMD, last_rc);
                send_message_string(cli_socket, (char *)io_buff);
                continue;
            case EXIT_SC:
                printf(RCMD_MSG_CLIENT_EXITED);
                free(io_buff);
                close(cli_socket);
                return OK;
            case STOP_SERVER_SC:
                printf(RCMD_MSG_SVR_STOP_REQ);
                free(io_buff);
                close(cli_socket);
                return OK_EXIT;
            default:
                break;
        }

        // TODO send_message_eof when done
        rc = send_message_eof(cli_socket);
        if (rc != OK){
            printf(CMD_ERR_RDSH_COMM);
            free(io_buff);
            close(cli_socket);
            return ERR_RDSH_COMMUNICATION;
        }

        printf(RCMD_MSG_SVR_EXEC_REQ, io_buff);

    }

    free(io_buff);
    close(cli_socket);
    free(cmd);
    for (int k=0; k < 20; k++){free(argList[k]);}
    free(argList);
    free(clist);
    return OK;
}

int send_message_eof(int cli_socket){
    int send_len = (int)sizeof(RDSH_EOF_CHAR);
    int sent_len;
    sent_len = send(cli_socket, &RDSH_EOF_CHAR, send_len, 0);

    if (sent_len != send_len){
        return ERR_RDSH_COMMUNICATION;
    }
    return OK;
}


int send_message_string(int cli_socket, char *buff){
    //TODO implement writing to cli_socket with send()
    int send_len = strlen(buff);
    int sent_len;
    sent_len = send(cli_socket, buff, send_len, 0);

    if (sent_len != send_len){
        return ERR_RDSH_COMMUNICATION;
    }
    return send_message_eof(cli_socket);
}

int rsh_execute_pipeline(int cli_sock, command_list_t *clist) 
{
    clist->num += 1;
    // if > character is found reduce the number of commands by one (so it doesn't try to execute the filename)
    if (clist->commands[0].argc <= -10 || clist->commands[0].argc >= 10){clist->num -= 1;}

    int pipes[clist->num - 1][2];// Array of pipes
    pid_t pids[clist->num];
    int  pids_st[clist->num];// Array to store process IDs
    Built_In_Cmds bi_cmd;
    int exit_code;
    int runExec = 1;

    
    // Create all necessary pipes
    for (int i = 0; i < clist->num - 1; i++)
     {
        if (pipe(pipes[i]) == -1) 
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < clist->num; i++) 
    {
        // TODO this is basically the same as the piped fork/exec assignment, except for where you connect the begin and end of the pipeline (hint: cli_sock)
        pids[i] = fork();
        if (pids[i] == -1)
         {
            perror("fork");
            exit(EXIT_FAILURE);
         }
        // TODO HINT you can dup2(cli_sock with STDIN_FILENO, STDOUT_FILENO, etc.
            // Child process
            if (pids[i] == 0) 
            {  
                runExec = 1;

                //if a redirect open the file as destination
                if (clist->commands[i].argc <= -10 || clist->commands[i].argc >= 10)
                {
                    char* filename = clist->commands[i + 1]._cmd_buffer;
                    int out = 0;
                    // if >>
                    if (clist->commands[i].argc == 20 || clist->commands[i].argc == -20)
                    {
                        out = open(filename, O_RDWR|O_CREAT|O_APPEND, 0600);
                    }
                    //if >
                    else if (clist->commands[i].argc == 10 || clist->commands[i].argc == -10)
                    {
                        out = open(filename, O_RDWR|O_CREAT|O_TRUNC, 0600);
                    }
                    //if error
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
                if (clist->commands[i].argc == -1 || clist->commands[i].argc == 1|| clist->commands[i].argc == 2|| clist->commands[i].argc == -2)
                {
                    // For first command in pipeline, read from socket unless input redirected
                    if (i == 0 && !clist->commands[i].input_file) {dup2(cli_sock, STDIN_FILENO);}

                    // For last command in pipeline, write to socket unless output redirected
                    if (i == clist->num - 1 && !clist->commands[i].output_file)
                    {
                        dup2(cli_sock, STDOUT_FILENO);
                        dup2(cli_sock, STDERR_FILENO);  // Also redirect stderr to socket
                    }
                    
                    // Set up input pipe for all except first process
                    if (i > 0) {dup2(pipes[i-1][0], STDIN_FILENO);}

                    // Set up output pipe for all except last process
                    if (i < clist->num - 1) {dup2(pipes[i][1], STDOUT_FILENO);}

                    // Close all pipe ends in child
                    for (int j = 0; j < clist->num - 1; j++) 
                    {
                        close(pipes[j][0]);
                        close(pipes[j][1]);
                    }

                    //if a local command execute but don't run execvp
                    if (strcmp(clist->commands[i]._cmd_buffer, "dragon") == 0)
                    {
                        printDragon();
                        runExec = 0;
                        exit(EXIT_FAILURE);
                    }
                    
                    if (strcmp(clist->commands[i]._cmd_buffer, "exit") == 0)
                    {
                        runExec = 0;
                        exit(EXIT_FAILURE);
                    }
                    
                    //if not a local command run execvp
                    if (runExec)
                    {
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
                            
                            //allocate and clear args memory
                            int numberArgs = clist->commands[i].argc;
                            char** args = malloc(numberArgs + 1 * sizeof(char));   
                            for(int k = 0; k <= numberArgs + 1; k++){args[k] =  malloc(sizeof(char) * (100));}     
                            for(int k = 0; k <= numberArgs + 1; k++){strcpy(args[k], "");}                             

                            //move arguments to args
                            for(int j = 1; j <= clist->commands[i].argc; j++){strcpy(args[j], clist->commands[i].argv[j-1]);}
                            
                            //move command to args 0
                            strcpy(args[0], clist->commands[i]._cmd_buffer);

                            //write NULL to last args entry
                            args[clist->commands[i].argc+1] = NULL;
                            
                            //print args
                            //for (int k = 0; k <= clist->commands[i].argc; k++){printf("args[%d]: %s (%d)\n", k, args[k], strlen(args[k]));}

                            execvp(args[0], args);
                            perror("execvp");
                            exit(EXIT_FAILURE);
                            //free args
                            for(int k = 0; k <= clist->commands[i].argc + 1; k++){free(args[k]);}
                            free(args);
                        }
                        
                    }
                    
                }
                
            }
    }


    // Parent process: close all pipe ends
    for (int i = 0; i < clist->num - 1; i++) 
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // Wait for all children
    for (int i = 0; i < clist->num; i++) 
    {
        waitpid(pids[i], &pids_st[i], 0);
    }

    //by default get exit code of last process
    //use this as the return value
    exit_code = WEXITSTATUS(pids_st[clist->num - 1]);
    for (int i = 0; i < clist->num; i++) 
    {
        //if any commands in the pipeline are EXIT_SC
        //return that to enable the caller to react
        if (WEXITSTATUS(pids_st[i]) == EXIT_SC)
            exit_code = EXIT_SC;
    }
    return exit_code;
}
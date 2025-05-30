
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

//INCLUDES for extra credit
//#include <signal.h>
//#include <pthread.h>
//-------------------------

#include "dshlib.h"
#include "rshlib.h"

static int is_threaded_server = false;

/*
 * start_server(ifaces, port, is_threaded)
 *      ifaces:  a string in ip address format, indicating the interface
 *              where the server will bind.  In almost all cases it will
 *              be the default "0.0.0.0" which binds to all interfaces.
 *              note the constant RDSH_DEF_SVR_INTFACE in rshlib.h
 * 
 *      port:   The port the server will use.  Note the constant 
 *              RDSH_DEF_PORT which is 1234 in rshlib.h.  If you are using
 *              tux you may need to change this to your own default, or even
 *              better use the command line override -s implemented in dsh_cli.c
 *              For example ./dsh -s 0.0.0.0:5678 where 5678 is the new port  
 * 
 *      is_threded:  Used for extra credit to indicate the server should implement
 *                   per thread connections for clients  
 * 
 *      This function basically runs the server by: 
 *          1. Booting up the server
 *          2. Processing client requests until the client requests the
 *             server to stop by running the `stop-server` command
 *          3. Stopping the server. 
 * 
 *      This function is fully implemented for you and should not require
 *      any changes for basic functionality.  
 * 
 *      IF YOU IMPLEMENT THE MULTI-THREADED SERVER FOR EXTRA CREDIT YOU NEED
 *      TO DO SOMETHING WITH THE is_threaded ARGUMENT HOWEVER.  
 */
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
        int err_code = svr_socket;  //server socket will carry error code
        return err_code;
    }
    
    rc = process_cli_requests(svr_socket);
    

    stop_server(svr_socket);


    return rc;
}

/*
 * stop_server(svr_socket)
 *      svr_socket: The socket that was created in the boot_server()
 *                  function. 
 * 
 *      This function simply returns the value of close() when closing
 *      the socket.  
 */
int stop_server(int svr_socket){
    return close(svr_socket);
}

/*
 * boot_server(ifaces, port)
 *      ifaces & port:  see start_server for description.  They are passed
 *                      as is to this function.   
 * 
 *      This function "boots" the rsh server.  It is responsible for all
 *      socket operations prior to accepting client connections.  Specifically: 
 * 
 *      1. Create the server socket using the socket() function. 
 *      2. Calling bind to "bind" the server to the interface and port
 *      3. Calling listen to get the server ready to listen for connections.
 * 
 *      after creating the socket and prior to calling bind you might want to 
 *      include the following code:
 * 
 *      int enable=1;
 *      setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
 * 
 *      when doing development you often run into issues where you hold onto
 *      the port and then need to wait for linux to detect this issue and free
 *      the port up.  The code above tells linux to force allowing this process
 *      to use the specified port making your life a lot easier.
 * 
 *  Returns:
 * 
 *      server_socket:  Sockets are just file descriptors, if this function is
 *                      successful, it returns the server socket descriptor, 
 *                      which is just an integer.
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code is returned if the socket(),
 *                               bind(), or listen() call fails. 
 * 
 */
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

    /*
     * NOTE this is good for development as sometimes port numbers
     * get held up, this forces the port to be bound, do not use
     * in a real application
     */
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

    /*
     * Prepare for accepting connections. The backlog size is set
     * to 20. So while one request is being processed other requests
     * can be waiting.
     */
    ret = listen(svr_socket, 20);
    if (ret == -1) {
        perror("listen");
        return ERR_RDSH_COMMUNICATION;
    }

    return svr_socket;
}

/*
 * process_cli_requests(svr_socket)
 *      svr_socket:  The server socket that was obtained from boot_server()
 *   
 *  This function handles managing client connections.  It does this using
 *  the following logic
 * 
 *      1.  Starts a while(1) loop:
 *  
 *          a. Calls accept() to wait for a client connection. Recall that 
 *             the accept() function returns another socket specifically
 *             bound to a client connection. 
 *          b. Calls exec_client_requests() to handle executing commands
 *             sent by the client. It will use the socket returned from
 *             accept().
 *          c. Loops back to the top (step 2) to accept connecting another
 *             client.  
 * 
 *          note that the exec_client_requests() return code should be
 *          negative if the client requested the server to stop by sending
 *          the `stop-server` command.  If this is the case step 2b breaks
 *          out of the while(1) loop. 
 * 
 *      2.  After we exit the loop, we need to cleanup.  Dont forget to 
 *          free the buffer you allocated in step #1.  Then call stop_server()
 *          to close the server socket. 
 * 
 *  Returns:
 * 
 *      OK_EXIT:  When the client sends the `stop-server` command this function
 *                should return OK_EXIT. 
 * 
 *      ERR_RDSH_COMMUNICATION:  This error code terminates the loop and is
 *                returned from this function in the case of the accept() 
 *                function failing. 
 * 
 *      OTHERS:   See exec_client_requests() for return codes.  Note that positive
 *                values will keep the loop running to accept additional client
 *                connections, and negative values terminate the server. 
 * 
 */
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
        
        // Process single threaded
        if(is_threaded_server == false)
        {
            rc = exec_client_requests(cli_socket);
            
            //if stop-server request stop program with rc = 0
            if (rc = -10)
            {
                rc = 0;
                break;
            }
            if (rc < 0){break;}
        } else 
        {
            rc = exec_client_thread(svr_socket, cli_socket);
            //if stop-server request stop program with rc = 0
            if (rc = -10)
            {
                rc = 0;
                break;
            }
            if (rc < 0){break;}
        }
    }

    stop_server(cli_socket);
    return rc;
}

/*
 * exec_client_requests(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client
 *   
 *  This function handles accepting remote client commands. The function will
 *  loop and continue to accept and execute client commands.  There are 2 ways
 *  that this ongoing loop accepting client commands ends:
 * 
 *      1.  When the client executes the `exit` command, this function returns
 *          to process_cli_requests() so that we can accept another client
 *          connection. 
 *      2.  When the client executes the `stop-server` command this function
 *          returns to process_cli_requests() with a return code of OK_EXIT
 *          indicating that the server should stop. 
 * 
 *  Note that this function largely follows the implementation of the
 *  exec_local_cmd_loop() function that you implemented in the last 
 *  shell program deliverable. The main difference is that the command will
 *  arrive over the recv() socket call rather than reading a string from the
 *  keyboard. 
 * 
 *  This function also must send the EOF character after a command is
 *  successfully executed to let the client know that the output from the
 *  command it sent is finished.  Use the send_message_eof() to accomplish 
 *  this. 
 * 
 *  Of final note, this function must allocate a buffer for storage to 
 *  store the data received by the client. For example:
 *     io_buff = malloc(RDSH_COMM_BUFF_SZ);
 *  And since it is allocating storage, it must also properly clean it up
 *  prior to exiting.
 * 
 *  Returns:
 * 
 *      OK:       The client sent the `exit` command.  Get ready to connect
 *                another client. 
 *      OK_EXIT:  The client sent `stop-server` command to terminate the server
 * 
 *      ERR_RDSH_COMMUNICATION:  A catch all for any socket() related send
 *                or receive errors. 
 */

 
 //_________________
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

void *handle_client(void *arg) { 
    thread_info_t *tinfo = (thread_info_t *)arg;
    int rc;

    //handles client requests in loop.
    rc = exec_client_requests(tinfo->client_socket);
    if (rc == OK){
        
        free(tinfo);        //was malloc'd
        //just return since this is a detached thread handler
        return NULL;
    } else {
        
        //unexpected error, force the overall process to close
        close(tinfo->server_socket);
        free(tinfo);        //was malloc'd
        exit(0);    //force the entire process to end
    }
}
//______________________


int exec_client_requests(int cli_socket) {
    
    
    //command_list_t cmd_list;
    //command_list_t *cmd_list = malloc(sizeof(command_list_t));
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
        
        //for (int k=0; k < sizeof(io_buff); k++){io_buff[k] = '0';}
        memset(io_buff, 0, RDSH_COMM_BUFF_SZ);
        // TODO use recv() syscall to get input

        io_size = recv(cli_socket, io_buff, RDSH_COMM_BUFF_SZ, 0);
        
        //printf("io_size: %d\n", io_size);
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
                        free(io_buff);
                        close(cli_socket);
                        free(cmd);
                        for (int k=0; k < 20; k++){free(argList[k]);}
                        free(argList);
                        free(clist);
                        return -10;
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

/*
 * send_message_eof(cli_socket)
 *      cli_socket:  The server-side socket that is connected to the client

 *  Sends the EOF character to the client to indicate that the server is
 *  finished executing the command that it sent. 
 * 
 *  Returns:
 * 
 *      OK:  The EOF character was sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the EOF character. 
 */
int send_message_eof(int cli_socket){
    int send_len = (int)sizeof(RDSH_EOF_CHAR);
    int sent_len;
    sent_len = send(cli_socket, &RDSH_EOF_CHAR, send_len, 0);

    if (sent_len != send_len){
        return ERR_RDSH_COMMUNICATION;
    }
    return OK;
}


/*
 * send_message_string(cli_socket, char *buff)
 *      cli_socket:  The server-side socket that is connected to the client
 *      buff:        A C string (aka null terminated) of a message we want
 *                   to send to the client. 
 *   
 *  Sends a message to the client.  Note this command executes both a send()
 *  to send the message and a send_message_eof() to send the EOF character to
 *  the client to indicate command execution terminated. 
 * 
 *  Returns:
 * 
 *      OK:  The message in buff followed by the EOF character was 
 *           sent successfully. 
 * 
 *      ERR_RDSH_COMMUNICATION:  The send() socket call returned an error or if
 *           we were unable to send the message followed by the EOF character. 
 */
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


/*
 * rsh_execute_pipeline(int cli_sock, command_list_t *clist)
 *      cli_sock:    The server-side socket that is connected to the client
 *      clist:       The command_list_t structure that we implemented in
 *                   the last shell. 
 *   
 *  This function executes the command pipeline.  It should basically be a
 *  replica of the execute_pipeline() function from the last deliverable. 
 *  The only thing different is that you will be using the cli_sock as the
 *  main file descriptor on the first executable in the pipeline for STDIN,
 *  and the cli_sock for the file descriptor for STDOUT, and STDERR for the
 *  last executable in the pipeline.  See picture below:  
 * 
 *      
 *┌───────────┐                                                    ┌───────────┐
 *│ cli_sock  │                                                    │ cli_sock  │
 *└─────┬─────┘                                                    └────▲──▲───┘
 *      │   ┌──────────────┐     ┌──────────────┐     ┌──────────────┐  │  │    
 *      │   │   Process 1  │     │   Process 2  │     │   Process N  │  │  │    
 *      │   │              │     │              │     │              │  │  │    
 *      └───▶stdin   stdout├─┬──▶│stdin   stdout├─┬──▶│stdin   stdout├──┘  │    
 *          │              │ │   │              │ │   │              │     │    
 *          │        stderr├─┘   │        stderr├─┘   │        stderr├─────┘    
 *          └──────────────┘     └──────────────┘     └──────────────┘   
 *                                                      WEXITSTATUS()
 *                                                      of this last
 *                                                      process to get
 *                                                      the return code
 *                                                      for this function       
 * 
 *  Returns:
 * 
 *      EXIT_CODE:  This function returns the exit code of the last command
 *                  executed in the pipeline.  If only one command is executed
 *                  that value is returned.  Remember, use the WEXITSTATUS()
 *                  macro that we discussed during our fork/exec lecture to
 *                  get this value. 
 */
int rsh_execute_pipeline(int cli_sock, command_list_t *clist) 
{
    clist->num += 1;
    // if > character is found reduce the number of commands by one (so it doesn't try to execute the filename)
    if (clist->commands[0].argc <= -10 || clist->commands[0].argc >= 10){clist->num -= 1;}

    //printf("execute pipeline\n");
    //printf("cmd list->num: %d\n", clist->num);
    //printf("cmd commands-> [0]: %s\n", clist->commands[0]._cmd_buffer);
    //printf("cmd args-> [0]: %s\n", clist->commands[0].argv);
    //printf("clist output file: %s\n", clist->commands[0].output_file);
    //printf("clist input file: %s\n", clist->commands[0].input_file);
    //printf("cli socket: %d\n", cli_sock);

    int pipes[clist->num - 1][2];  // Array of pipes
    pid_t pids[clist->num];
    int  pids_st[clist->num];         // Array to store process IDs
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
                            
                            int j = 1;
                            char** args = malloc(sizeof(char) * clist->commands[i].argc);
                            
                            for(j = 0; j <= clist->commands[i].argc + 1; j++)
                            {
                                args[j] =  malloc(sizeof(char)*50);
                            }

                            strcpy(args[0],clist->commands[i]._cmd_buffer);
                            
                            for(j = 1; j <= clist->commands[i].argc; j++)
                            {
                                strcpy(args[j], clist->commands[i].argv[j-1]);
                                printf("args[%d]: %s\n",j, args[j]);
                            }
                            
                            args[clist->commands[i].argc+1] = NULL;
                            
                            execvp(args[0], args);
                            perror("execvp");
                            exit(EXIT_FAILURE);
                        }
                        //else if (clist->commands[i].argc > 0)
                        //{
                        //    char *args[] = {clist->commands[i]._cmd_buffer, clist->commands[i].argv, NULL};
                        //    execvp(args[0], args);
                        //    perror("execvp");
                        //    exit(EXIT_FAILURE);
                        //}
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

/**************   OPTIONAL STUFF  ***************/
/****
 **** NOTE THAT THE FUNCTIONS BELOW ALIGN TO HOW WE CRAFTED THE SOLUTION
 **** TO SEE IF A COMMAND WAS BUILT IN OR NOT.  YOU CAN USE A DIFFERENT
 **** STRATEGY IF YOU WANT.  IF YOU CHOOSE TO DO SO PLEASE REMOVE THESE
 **** FUNCTIONS AND THE PROTOTYPES FROM rshlib.h
 **** 
 */

/*
 * rsh_match_command(const char *input)
 *      cli_socket:  The string command for a built-in command, e.g., dragon,
 *                   cd, exit-server
 *   
 *  This optional function accepts a command string as input and returns
 *  one of the enumerated values from the BuiltInCmds enum as output. For
 *  example:
 * 
 *      Input             Output
 *      exit              BI_CMD_EXIT
 *      dragon            BI_CMD_DRAGON
 * 
 *  This function is entirely optional to implement if you want to handle
 *  processing built-in commands differently in your implementation. 
 * 
 *  Returns:
 * 
 *      BI_CMD_*:   If the command is built-in returns one of the enumeration
 *                  options, for example "cd" returns BI_CMD_CD
 * 
 *      BI_NOT_BI:  If the command is not "built-in" the BI_NOT_BI value is
 *                  returned. 
 

 
Built_In_Cmds rsh_match_command(const char *input)
{
    if (strcmp(input, "exit") == 0)
        return BI_CMD_EXIT;
    if (strcmp(input, "dragon") == 0)
        return BI_CMD_DRAGON;
    if (strcmp(input, "cd") == 0)
        return BI_CMD_CD;
    if (strcmp(input, "stop-server") == 0)
        return BI_CMD_STOP_SVR;
    if (strcmp(input, "rc") == 0)
        return BI_CMD_RC;
    return BI_NOT_BI;
}


 * rsh_built_in_cmd(cmd_buff_t *cmd)
 *      cmd:  The cmd_buff_t of the command, remember, this is the 
 *            parsed version fo the command
 *   
 *  This optional function accepts a parsed cmd and then checks to see if
 *  the cmd is built in or not.  It calls rsh_match_command to see if the 
 *  cmd is built in or not.  Note that rsh_match_command returns BI_NOT_BI
 *  if the command is not built in. If the command is built in this function
 *  uses a switch statement to handle execution if appropriate.   
 * 
 *  Again, using this function is entirely optional if you are using a different
 *  strategy to handle built-in commands.  
 * 
 *  Returns:
 * 
 *      BI_NOT_BI:   Indicates that the cmd provided as input is not built
 *                   in so it should be sent to your fork/exec logic
 *      BI_EXECUTED: Indicates that this function handled the direct execution
 *                   of the command and there is nothing else to do, consider
 *                   it executed.  For example the cmd of "cd" gets the value of
 *                   BI_CMD_CD from rsh_match_command().  It then makes the libc
 *                   call to chdir(cmd->argv[1]); and finally returns BI_EXECUTED
 *      BI_CMD_*     Indicates that a built-in command was matched and the caller
 *                   is responsible for executing it.  For example if this function
 *                   returns BI_CMD_STOP_SVR the caller of this function is
 *                   responsible for stopping the server.  If BI_CMD_EXIT is returned
 *                   the caller is responsible for closing the client connection.
 * 
 *   AGAIN - THIS IS TOTALLY OPTIONAL IF YOU HAVE OR WANT TO HANDLE BUILT-IN
 *   COMMANDS DIFFERENTLY. 
 
Built_In_Cmds rsh_built_in_cmd(cmd_buff_t *cmd)
{
    Built_In_Cmds ctype = BI_NOT_BI;
    ctype = rsh_match_command(cmd->argv[0]);

    switch (ctype)
    {
    // case BI_CMD_DRAGON:
    //     print_dragon();
    //     return BI_EXECUTED;
    case BI_CMD_EXIT:
        return BI_CMD_EXIT;
    case BI_CMD_STOP_SVR:
        return BI_CMD_STOP_SVR;
    case BI_CMD_RC:
        return BI_CMD_RC;
    case BI_CMD_CD:
        chdir(cmd->argv[1]);
        return BI_EXECUTED;
    default:
        return BI_NOT_BI;
    }
}
*/

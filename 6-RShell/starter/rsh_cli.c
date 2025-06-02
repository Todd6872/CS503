
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>
#include "dshlib.h"
#include "rshlib.h"


int exec_remote_cmd_loop(char *address, int port)
{
    char *cmd_buff;
    char *rsp_buff;
    int cli_socket;
    ssize_t io_size;
    int is_eof;

    cmd_buff = malloc(SH_CMD_MAX);
    rsp_buff = malloc(RDSH_COMM_BUFF_SZ);

    cli_socket = start_client(address,port);
    if (cli_socket < 0){
        perror("start client");
        return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_CLIENT);
    }

    while (1) 
    {
        // TODO print prompt
        printf("%s", SH_PROMPT);

        // TODO fgets input
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL)
        {
            printf("\n");
            break;
        }
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        if (cmd_buff[0] == '\0'){continue;}

        // TODO send() over cli_socket
        int send_len = strlen(cmd_buff) + 1;
        io_size = send(cli_socket, cmd_buff, send_len,0);
        // TODO break on exit command
        if (strcmp(cmd_buff, "exit") == 0){return 0;}

        if (io_size < 0)
        {
            perror("write to backend server failed");
            return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_COMMUNICATION);
        }

        
        if (io_size != send_len){
            printf(CMD_ERR_RDSH_SEND, (int)io_size, (int)send_len);
            return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_COMMUNICATION);
        }

        // TODO recv all the results
        while ((io_size = recv(cli_socket, rsp_buff, RDSH_COMM_BUFF_SZ,0)) > 0) {
            //see if the last character is an eof
            is_eof = ((char)rsp_buff[io_size-1] == RDSH_EOF_CHAR) ? 1 : 0;

            if(is_eof)
                rsp_buff[io_size-1] = '\0';  //mark it as null to string terminate

            //print data
            printf("%.*s", (int)io_size, rsp_buff);

            if (is_eof)
                break;
        }

        if (io_size == 0){
            printf(RCMD_SERVER_EXITED);
            return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_COMMUNICATION);
        }
        if (io_size < 0){
            perror("network read error");
            return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_COMMUNICATION);
        }

        
    }

    return client_cleanup(cli_socket, cmd_buff, rsp_buff, OK);
}

int start_client(char *server_ip, int port){
    struct sockaddr_in addr;
    int cli_socket;
    int ret;

    // TODO set up cli_socket
    cli_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (cli_socket == -1) {
        perror("socket");
        return ERR_RDSH_CLIENT;
    }

    memset(&addr, 0, sizeof(struct sockaddr_in));

    // Connect socket to socket address
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(server_ip);
    addr.sin_port = htons(port);

    ret = connect (cli_socket, (const struct sockaddr *) &addr,
                   sizeof(struct sockaddr_in));
    if (ret == -1) {
        fprintf(stderr, "The server is down.\n");
        return ERR_RDSH_CLIENT;
    }


    return cli_socket;
}

int client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int rc){
    //If a valid socket number close it.
    if(cli_socket > 0){
        close(cli_socket);
    }

    //Free up the buffers 
    free(cmd_buff);
    free(rsp_buff);

    //Echo the return value that was passed as a parameter
    return rc;
}
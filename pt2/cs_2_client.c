/////////////////////////////////////////
// Danielle Wisemiller
// OS - part two
// 15 April 2018
//////////////////////////////////////////

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#define MAX_BUF 1024
#define MSG_LENGTH 150 // maximum length of message

//types of command for return or commands
typedef enum {
  REGULAR,
  COMMAND
} msg_type_t;

//types of commands from prev enum
typedef enum {
	SEND, EXIT,
	STATUS, TIME, 
	RET_ST, RET_TI
} cmd_type_t;

//initial request struct
typedef struct msg_init {
  int pid_num;
} pid_sent_t;

//child request struct 
typedef struct msg_request {
  msg_type_t type; //reg or command
  cmd_type_t subtype; //send rec time status
  int status;
  char message_text[MSG_LENGTH];
} messages_1_t;

int main()
{
    //declare variables for pipes and buffers 
	int fd, fd_send, fd_rec;
    char buf[MAX_BUF];
	char buf1[MAX_BUF];
	char buf2[MAX_BUF];
    char *server_fifo = "/tmp/server_np";
    //write PID to the server FIFO as initial request
    fd = open(server_fifo, O_WRONLY);
	
	//send initial request of pid to server
	pid_sent_t initial_1;
	initial_1.pid_num = getpid();
	if( write(fd, &initial_1, sizeof(pid_sent_t)) < 0)
        fprintf(stderr, "Error\n");
    close(fd);
	
	//format send & rec pipes
	sprintf(buf1, "/tmp/%s_send", buf);
	char *fifoSend = buf1;
	mkfifo(fifoSend, 0666);
	
	sprintf(buf2, "/tmp/%s_receive", buf);
	char *fifoRec = buf2;
	mkfifo(fifoRec, 0666);

	//read input from terminal for message requests
	while(1)
	{
		//open pipe and declare variables
		fd_send = open(fifoSend, O_WRONLY);
		char *cmd = NULL;
		size_t bufsize = MAX_BUF;
        char str[MSG_LENGTH];
		struct msg_request messages_1;
		
		//get command type
		printf("Enter a command starting with 'send:' or 'exit' \n");
		scanf("%s", str);
		
		//send command
		if (strstr(str, "send:") != NULL || strstr(str, "Send:") != NULL)
		{
			//get the message from the send command
			size_t characters = getline(&cmd,&bufsize,stdin); // Get the command
			cmd[strlen(cmd)-1] = '\0'; // Remove trailing newline
			memmove(str, str+1, strlen(str));
			//set values in struct
			messages_1.type = COMMAND;
			messages_1.subtype = SEND;
			strcpy(messages_1.message_text, cmd);
			//write struct to server
		    if(write(fd_send, &messages_1, sizeof(messages_1_t)) < 0) 
			{
		        fprintf(stderr, "Error\n");
				close(fd_send);
			} 
			else 
			{
				// Wrote to the server correctly
				close(fd_send);
			}
		}
		//status command
		else if (strstr(str, "status") != NULL || strstr(str, "Status") != NULL)
		{
			//set values in struct
			messages_1.type = COMMAND;
			messages_1.subtype = STATUS;
			strcpy(messages_1.message_text, str);
			//write to server
			if(write(fd_send, &messages_1, sizeof(messages_1_t)) < 0)
			{
		        fprintf(stderr, "Error\n");
				close(fd_send);
			}
			//get status back from server after written
			else
			{
				// Wrote to the server correctly
				close(fd_send);
				//open rec pipe
				fd_rec = open(fifoRec, O_RDONLY);
				messages_1_t ret_status;
				//get struct back from server with status value
				while(read(fd_rec, &ret_status, sizeof(messages_1_t))>0)
				{
					if(ret_status.subtype == RET_ST)
					{
						//print status to client terminal
						printf("%s%d\n", ret_status.message_text, ret_status.status);
					}
					close(fd_rec);
				}
			}
		}
		//time command
		else if (strstr(str, "time") != NULL || strstr(str, "Time") != NULL)
		{
			//set values for struct
			messages_1.type = COMMAND;
			messages_1.subtype = TIME;
			strcpy(messages_1.message_text, str);
			//send struct to server
			if(write(fd_send, &messages_1, sizeof(messages_1_t)) < 0)
			{
		        fprintf(stderr, "Error\n");
				close(fd_send);
			} 
			//get time back from server
			else 
			{
				// Wrote to the server correctly
				
				close(fd_send);
				//open pipe to get struct
				fd_rec = open(fifoRec, O_RDONLY);
				messages_1_t ret_time;
				//read from server
				while(read(fd_rec, &ret_time, sizeof(messages_1_t))>0)
				{
					if(ret_time.subtype == RET_TI)
					{
						//print time to client terminal
						printf("%s\n", ret_time.message_text);
					}
					close(fd_rec);
				}
			}
		}
		//exit command
		else if (strstr(str, "exit") != NULL || strstr(str, "Exit") != NULL)
		{
			//set values of struct
			messages_1.type = COMMAND;
			messages_1.subtype = EXIT;
			strcpy(messages_1.message_text, str);
			//write to server
			if(write(fd_send, &messages_1, sizeof(messages_1_t)) < 0)
			{
		        fprintf(stderr, "Error\n");
				close(fd_send);
			}
			else
			{
				// Wrote to the server correctly
				close(fd_send);
			}
			//exit the process
			exit(0);
		}
		//catch all for invalid inputs
		else
		{
			printf("%s%s", str, "was inputted without correct command\n");	
		}	
		
	}
	
    return 0;
}
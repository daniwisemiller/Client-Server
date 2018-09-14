/////////////////////////////////////////
// Danielle Wisemiller
// OS - part two
// 15 April 2018
//////////////////////////////////////////

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>


#define MAX_BUF 1024
#define MSG_LENGTH 150 // maximum length of message
int processes = 0; //to count the child processes

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

//to decrement the child processes 
void decrement_handler(int signal)
{
	pid_t pid;
	pid = wait(NULL);
	processes--;
}
//to unlink the fifo after ctrl + c to end server process
void quit_handler(int signal)
{
	char *server_fifo = "tmp/server_np";
	unlink(server_fifo);
	exit(0);
}

int main()
{
    //initialization of pipe and buffers
	int fd, fd_send, fd_rec;
    char *server_fifo = "/tmp/server_np";
    char buf[MAX_BUF];
	char sent_pid[MAX_BUF];
	
	int server_pid = getppid();
    /* create the FIFO (named pipe) */
    mkfifo(server_fifo, 0666);
	int fork_return;

	//to catch child process ending and ctrl + c to quit server
	signal(SIGCHLD, decrement_handler);
	signal(SIGINT, quit_handler);
    /* open, read, and display the pid from the FIFO */
    while(1 && getppid() == server_pid)
    {	
		// check for clients sending info
        fd = open(server_fifo, O_RDONLY);
		pid_sent_t initial_1;
		
		//read pid sent by client
        while(read(fd, &initial_1, sizeof(pid_sent_t))>0)
        {
            printf("Received: %i\n", initial_1.pid_num);
			sprintf(sent_pid, "%i", initial_1.pid_num);
			processes+=1;
			fork_return = fork();
        }
		//check for fork return -> child
		if(fork_return == 0)
		{
			//create path for pipes
			char buf1[MAX_BUF];
			sprintf(buf1, "/tmp/%s_send", buf);
			char *fifoSend = buf1;
	
			char buf2[MAX_BUF];
			sprintf(buf2, "/tmp/%s_receive", buf);
			char *fifoRec = buf2;
	
		    while(1)
		    {
				// check for client sending
		        fd_send = open(fifoSend, O_RDONLY);				
				struct msg_request messages_1;
				//read struct sent by client
		        while(read(fd_send, &messages_1, sizeof(messages_1_t))>0)
		        {
					if (messages_1.type == COMMAND)
					{
						//check for status command
						if(messages_1.subtype == STATUS)
						{
							close(fd_send);
							//send the status back to the client
							fd_rec = open(fifoRec, O_WRONLY);														
							//struct variables being set
							messages_1_t return_status;
							return_status.subtype = RET_ST;
							return_status.status = processes;
						    strcpy(return_status.message_text, "Num of processes:");
							//writing struct back to client
						    if( write(fd_rec, &return_status, sizeof(messages_1_t)) < 0)
						        fprintf(stderr, "Error\n");
							else								
								close(fd_rec);
						}
						//check for time command
						else if (messages_1.subtype == TIME)
						{
							//printf("%u\n", messages_1.subtype);
							close(fd_send);
							
							//get time of day
							time_t curtime;
							struct tm *loc_time;
							//Getting current time of system
							curtime = time (NULL);
							loc_time = localtime (&curtime);
							//open pipe and set struct
							fd_rec = open(fifoRec, O_WRONLY);														
							messages_1_t return_time;
							return_time.subtype = RET_TI;
						    strftime (return_time.message_text, 150, "Time is %I:%M %p.", loc_time);
	
						    //write to client
						    if( write(fd_rec, &return_time, sizeof(messages_1_t)) < 0)
						        fprintf(stderr, "Error\n");
							else								
								close(fd_rec);
						}
						//check for exit command and exit child
						else if (messages_1.subtype == EXIT)
						{
				            printf("Received: %s\n", messages_1.message_text);
							_exit(0);
					
						}
						//print received string message from pt1
						else if (messages_1.subtype == SEND)
						{
							printf("server (P: %d) received string %s from client (P: %d)\n", getpid(), messages_1.message_text, getppid());
						}				
					}
				
		        }
		        close(fd_send);
		    }
			return(0);
		}
		//parent fork return
		else
			continue;
        close(fd);
    }
    /* remove the FIFO */
    unlink(server_fifo);

    return 0;
}
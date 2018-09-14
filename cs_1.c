//******************
//Danielle Wisemiller
//Client - Server pt 1
//04/02/2018
//******************

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define READ 0
#define WRITE 1

#define MSG_LENGTH 100 // maximum length of message

typedef enum {
  REGULAR,
  COMMAND
} msg_type_t;

typedef struct msg {
  msg_type_t type;
  char message_text[MSG_LENGTH];
} msg_t;

int main()
{
	int fd[2];
	pipe(fd);
	pid_t pid;

	struct msg messages_1;
	pid = fork();

	if(pid < 0) /* Fork failed - print error */ 
	{
		printf("ERROR\n");
		exit(0);
	} 
	else if (pid == 0) /* Child */ 
	{
		close(fd[READ]); /* close unused end */ 
		
		//declare variables
		char *input_str;
		size_t buffer = 32;
		size_t length;
        char str[MSG_LENGTH];
		//prompt user for input
        while(1)
        {
		    printf("input line of text. starting with \'send:\' or \'exit\': \n");
			
			//read input and check for commands
        	scanf("%s", str);
	        if (strstr(str, "send:") != NULL || strstr(str, "Send:") != NULL)
	        {
				//read message from input
				length = getline(&input_str, &buffer, stdin);
				input_str[strlen(input_str)-1] = '\0';
				memmove(input_str, input_str+1, strlen(input_str));
				
				//set input to struct and write to server
				messages_1.type = REGULAR;
				strcpy(messages_1.message_text, input_str);
				write(fd[WRITE], messages_1.message_text, length+1);
	        }
	        else
	        {
				//set input to struct and write to server
				messages_1.type = COMMAND;
				strcpy(messages_1.message_text, "exit");
		        write(fd[WRITE], messages_1.message_text, strlen("exit")+1);
		        close(fd[WRITE]);
				_exit(0);
	        }
      
		}
	} 
	else /* Parent */
	{
		close(fd[WRITE]); /* close unused end */

		while(1)
		{
			//read info from client
	        read(fd[READ], messages_1.message_text, MSG_LENGTH); 
			//check for command
	        if (strstr(messages_1.message_text, "exit") != NULL || strstr(messages_1.message_text, "Exit") != NULL)
	        {
				//print string then exit
	        	printf("server (P: %d) exits\n", getpid());
				close (fd[READ]); /* close original used end */
	        	exit(0);
	        }
	        else 
	        {
				//print input string then continue receiving input
				printf("server (P: %d) received string %s from client (P: %d)\n", getpid(), messages_1.message_text, pid);
			} 
		}
	} 
}
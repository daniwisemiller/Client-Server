#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

int main()
{
  int welcomeSocket, newSocket, clientSocket;
  char buffer[1024];
  struct sockaddr_in serverAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;

  /*---- Create the socket. The three arguments are: ----*/
  /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
  welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);
  
  /*---- Configure settings of the server address struct ----*/
  /* Address family = Internet */
  serverAddr.sin_family = AF_INET;
  /* Set port number, using htons function to use proper byte order */
  serverAddr.sin_port = htons(7891);
  /* Set IP address to localhost */
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  /* Set all bits of the padding field to 0 */
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

  /*---- Bind the address struct to the socket ----*/
  bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

  pid_t pid = fork();
  
 
  if (pid > 0)
  {
      printf("Parent\n");

  }
  else if (pid == 0)
  {
	  /*---- Connect the socket to the server using the address struct ----*/
	  addr_size = sizeof serverAddr;
	  connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size);

	  addr_size = sizeof serverStorage;
	  newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);

	  strcpy(buffer,"Hello World\n");
	  send(newSocket,buffer,13,0);
  
	  /*---- Read the message from the server into the buffer ----*/
	  recv(clientSocket, buffer, 1024, 0);

	  /*---- Print the received message ----*/
	  printf("Data received: %s",buffer);   
  }
 



  return 0;
}

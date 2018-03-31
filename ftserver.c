/*Nathan Greenlaw
 * CS 372
 * Co-opted from my Program 4: otp_dec_d decrypt text with a given key by receiving it from otp_dec from cs 344
 * ftserver.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/ioctl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

void error(const char *msg) { fprintf(stderr,"%s\n",msg); exit(1);}//perror(msg); exit(1); } // Error function used for reporting issues

//realloc memory takes the string to resize and then resizes it to the given value = strlen(string)+value given in then the function call
void resizeString(char* stringR, int value)
{
	stringR = realloc(stringR, (value+1)*sizeof(char));
}

//Sends a message and takes the socket, the message and the length of the message using string length
void sendMessage(int serverSocket, char* msg, int bytesLeft)
{
	int charsWritten;
	int curBuffer = 0;
	int i=0;
	int left = bytesLeft;
	char* nextChar = msg;
	while(curBuffer < bytesLeft)
	{
		charsWritten = send(serverSocket, nextChar,left,0);
		if(charsWritten < 0)
		{error("unable to write to sock");exit(1);}
		i = curBuffer;
		curBuffer += charsWritten;
		left -= charsWritten;
		while(i < curBuffer)
		{
			nextChar++;
			i++;
		}
	}
	
}

//Receives a message and takes the socket and the length of the message using string length
void recvMessage(int serverSocket, char* msg, int bytesLeft)
{
	int charsWritten;
	int curBuffer = 0;
	int i=0;
	int left = bytesLeft;
	char* nextChar = msg;
	while(curBuffer < bytesLeft)
	{
		charsWritten = recv(serverSocket, nextChar,left,0);
		if(charsWritten < 0)
		{error("unable to write to sock");exit(1);}
		i = curBuffer;
		curBuffer += charsWritten;
		left -= charsWritten;
		while(i < curBuffer)
		{
			nextChar++;
			i++;
		}
	}
}

//Receives a message and takes the socket and the length of the message using string length and uses the terminal characters @@
void recvMessageAt(int serverSocket, char* msg)
{
	int charsWritten;
	int curBuffer = 0;
	int i=0;
	char* nextChar = msg;
	char readBuffer[10];
	while(strstr(msg, "@@") == NULL)
	{
		charsWritten = recv(serverSocket, readBuffer,sizeof(readBuffer)-1,0);
		if(charsWritten < 0)
		{error("unable to write to sock");exit(1);}
		curBuffer += charsWritten;
		strcpy(nextChar, readBuffer);
		while(i < curBuffer)
		{
			nextChar++;
			i++;
		}
	}
}

int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	//char buffer[256];
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) {error("ERROR opening socket"); exit(2);}

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		{error("ERROR on binding");exit(1);}
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	printf("Server open on %d\n",portNumber);

//loop until the server is killed
while(1)
{

	// Accept a connection, blocking if one is not available until one connects
	sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
	establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
	if (establishedConnectionFD < 0) {error("ERROR on accept");exit(1);}

	printf("Client Connected\n");

	//allocate memory for the send and recv messages
	char* sendMsg = malloc((2048+1)*sizeof(char));
	memset(sendMsg, '\0', sizeof(sendMsg));
	char* recvMsg = malloc((2048+1)*sizeof(char));
	memset(recvMsg, '\0', sizeof(recvMsg));
	
	//printf("Recving Com length\n");

	//receive length of command using a terminal character for the first one
	recvMessageAt(establishedConnectionFD, recvMsg);
	int comLength = atoi(recvMsg);
	char* msgLen = malloc((2048+1)*sizeof(char));
	memset(msgLen, '\0', sizeof(msgLen));
	int code;
	
	//printf("Recved com length and Sending Ready\n");
	
	//send ready
	char* ready = malloc(6*sizeof(char));
	memset(ready, '\0', sizeof(ready));
	strcpy(ready,"ready");
	sendMessage(establishedConnectionFD, ready, strlen(ready));
	
	//printf("Sent Ready\n");
	
	//receive command

	//printf("recving command\n");	

	recvMessage(establishedConnectionFD, recvMsg, comLength);
	
	printf("Received command %s\n", recvMsg);
	//process command
	char* directoryString = malloc(2048*sizeof(char));
	memset(directoryString, '\0', sizeof(directoryString));
	if(strstr(recvMsg, "-l"))
	{
		code = 0;
		//get directory stuff
		DIR *d;
		struct dirent *dir;
		d = opendir(".");
		if (d) 
		{
			if((dir = readdir(d)) != NULL)
			{
				strcpy(directoryString, dir->d_name);
				resizeString(directoryString, (strlen(dir->d_name)+1));
				strcat(directoryString, " ");
			}
			while ((dir = readdir(d)) != NULL)
			{
				resizeString(directoryString, (1+strlen(directoryString)+strlen(dir->d_name)));
				strcat(directoryString, dir->d_name);
				strcat(directoryString, " ");
			}
			closedir(d);
		}
		resizeString(sendMsg, strlen(directoryString));
		strcpy(sendMsg, directoryString);
		//printf("dir: %s\n", sendMsg);
	}
	

	//learned about fstat here : https://stackoverflow.com/questions/19328393/fstat-returning-0-file-size
	else
	{
		//Memmap found here: https://stackoverflow.com/questions/2151407/how-should-i-store-a-large-amout-of-text-data-in-memory
		//For large file sizes this is not a good way to do it
		struct stat fileStat;
		char* buffer;
		char* fileNameF = strtok(recvMsg, " ");
		if(strcmp(fileNameF, "-g") == 0)
		{
			code = 1;
			fileNameF = strtok(NULL," ");
			printf("Sending file: %s\n",fileNameF);
			FILE *f = fopen(fileNameF,"rt");
			int fd = 0;
			fd = open(fileNameF,O_RDONLY);
			fstat(fd, &fileStat);
			if(f != NULL)
			{
				off_t length = fileStat.st_size;
				buffer = mmap(0,length,PROT_READ,MAP_SHARED, fd, 0);
				/*printf("FSIZE: %d\n",length);
				char* buffer = malloc((length+1));
				memset(buffer,'\0',sizeof(buffer));
				fread(buffer,1,length,f);*/
				fclose(f);
				close(fd);
				resizeString(sendMsg, strlen(buffer));
				strcpy(sendMsg, buffer);
				munmap(buffer, length);
				//free(buffer);
			}
			
			//if file doesnt exist
			else
			{
				printf("File not found, sending error\n");
				strcpy(sendMsg, "File does Not Exist Error");
				code = 2;
			}
		}
		
		else
		{
			printf("Incorrect Command Given\n");
			strcpy(sendMsg, "Incorrect Command Error");
			code = 2;
		}
	}
	
	//get the length of the message and store it
	sprintf(msgLen, "%d", strlen(sendMsg));
	strcat(msgLen, "@@");
	resizeString(msgLen, (strlen(msgLen)));
	
	//printf("Sending msgLen %s\n", msgLen);
	
	//send length of message
	sendMessage(establishedConnectionFD, msgLen, strlen(msgLen));
	
	//printf("Sent msgLen %d\n", strlen(msgLen));
	//receive ready
	//printf("RECVing READY\n");
	recvMessage(establishedConnectionFD, recvMsg, strlen(ready));
	
	//printf("Recved ready\n");
	//printf("Sending Message\n");
	//send message
	sendMessage(establishedConnectionFD, sendMsg, strlen(sendMsg));
	
	//printf("Sent Message\n");
	//close client socket
	close(establishedConnectionFD); // Close the existing socket which is connected to the client
	
	printf("Socket Closed\n");	

	//Free the memory
	free(sendMsg);
	free(recvMsg);
	free(msgLen);
	free(directoryString);
	free(ready);

}
	close(listenSocketFD); // Close the listening socket
	return 0; 
}

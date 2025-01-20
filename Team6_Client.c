#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 

#define PORT 8081
 
int main(int argc, char const *argv[]) 
{ 
	int sock = 0, valread; 
	struct sockaddr_in serv_addr; 
	char buffer[1024] = {0}; 

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 
 
	serv_addr.sin_family = AF_INET;		
	serv_addr.sin_port = htons(PORT);	// uint16_t port number, network byte order 
 
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
	{ 
		printf("\nInvalid address/ Address not supported \n"); 
		return -1; 
	} 
 
	char *line = NULL;
	size_t len = 0;
	
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
		return -1; 
	} 

	// input lines and print
	while(1)
	{
		memset(buffer, 0, sizeof(buffer));
		getline(&line, &len, stdin);
		write(sock, line, len);
		valread = read(sock , buffer, 1024);
		printf("Server (Received) : %s\n", buffer);		
	}

	return 0; 
} 

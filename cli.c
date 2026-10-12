#include <netinet/in.h> //structure for storing address information 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <sys/socket.h> //for socket APIs 
#include <sys/types.h> 
#include <unistd.h>
  
#define PORT 9001
#define MAX_COMMAND_LINE_LEN 1024

char* getCommandLine(char *command_line){

	do{ 

            // Read input from stdin and store it in command_line. If there's an
            // error, exit immediately. (If you want to learn more about this line,
            // you can Google "man fgets")
        
            if ((fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) && ferror(stdin)) {
                fprintf(stderr, "fgets error");
                exit(0);
            }
 
        }while(command_line[0] == 0x0A);  // while just ENTER pressed
        command_line[strlen(command_line) - 1] = '\0';
				return command_line;
}

int main(int argc, char const* argv[]) 
{ 
    int sockID = socket(AF_INET, SOCK_STREAM, 0); 
    char  *token, *cp;
    char buf[MAX_COMMAND_LINE_LEN];
		char responeData[MAX_COMMAND_LINE_LEN];

    struct sockaddr_in servAddr; 
    memset(&servAddr, 0, sizeof(servAddr));
    memset(responeData, '\0', sizeof(responeData));
  
    servAddr.sin_family = AF_INET; 
    servAddr.sin_port 
        = htons(PORT); // use some unused port number 
    servAddr.sin_addr.s_addr = INADDR_ANY; 
  
    int connectStatus 
        = connect(sockID, (struct sockaddr*)&servAddr, 
                  sizeof(servAddr)); 
  
    if (connectStatus == -1) { 
        printf("Error...\n"); 
    } 
    else { 
  
			while(1) {
			  printf("Enter Command (or menu): ");
        getCommandLine(buf);

				send(sockID, buf, strlen(buf), 0);  // send command and args to server

				cp = buf;
        token = strtok(cp, " ");

				if(strcmp(token,"exit") == 0){
					close(sockID);
					exit(0);
				}
				else if(strcmp(token,"menu") == 0){
					printf("COMMANDS:\n---------\n1. print\n2. get_length\n3. add_back <value>\n4. add_front <value>\n5. add_position <index> <value>\n6. remove_back\n7. remove_front\n8. remove_position <index>\n9. get <index>\n10. exit\n");
				}
 
        int r = recv(sockID, responeData, sizeof(responeData) - 1, 0); // receive response from server
        if (r <= 0) { printf("\nServer closed the connection.\n"); break; }
        responeData[r] = '\0';
  
        printf("\nSERVER RESPONSE: %s\n", responeData); 
				memset(buf, '\0', MAX_COMMAND_LINE_LEN);
			}
    } 
 
    return 0; 
}
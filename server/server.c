#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
    int server_sockfd, client_sockfd;
    int state, client_len;
    size_t fsize = 0, nsize = 0, fpsize = 0;
    size_t fsize2 = 0;
    size_t fsize3 = 0;
    FILE *file = NULL;

    int ret;
    struct stat file_stat;

    struct sockaddr_in clientaddr, serveraddr;

    char buf[256];
    memset(buf, 0x00, 256);
    state = 0;

    client_len = sizeof(clientaddr);

    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error : ");
        exit(0);
    }

    memset(&serveraddr,0x00, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(8002);

    state = bind(server_sockfd , (struct sockaddr *)&serveraddr,
            sizeof(serveraddr));
    if (state == -1) {
        perror("bind error : ");
        exit(0);
    }
    state = listen(server_sockfd, 5);
    if (state == -1) {
        perror("listen error : ");
        exit(0);
    }

	client_sockfd = accept(server_sockfd, (struct sockaddr *)&clientaddr, &client_len);


	// while loop as much as the number of file
	while(1) {
		// file name transfer
	    char* fileName = "example.wav";
	    int fileNameSize = strlen(fileName);

	    printf("fileNameSize is %d\n", strlen(fileName));
	    printf("fileName is %s\n", fileName);

	    send(client_sockfd, &fileNameSize, sizeof(fileNameSize), 0);
	    send(client_sockfd, fileName, strlen(fileName), 0);

	    // file transfer
	    file = fopen("example.wav", "rb");
	    fseek(file, 0, SEEK_END);
	    fsize = ftell(file);
	    fseek(file, 0, SEEK_SET);

	    printf("file size [%d]\n", fsize);
	    fsize2 = htonl(fsize);
	    printf("file size [%d]\n", fsize2);

	    send(client_sockfd, &fsize, sizeof(fsize), 0);

	    while(nsize != fsize)
	    {
	        fpsize = fread(buf, 1, 256, file);
	        nsize = nsize+fpsize;
	        send(client_sockfd, buf, fpsize, 0);
	    }
	    printf("file send \n");
	    fclose(file);

	    // break because the number of file is 1
	    break;
	}




    close(client_sockfd);

    return 0;
}

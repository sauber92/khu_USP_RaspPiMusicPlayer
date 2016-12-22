#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wiringPi.h>			// raspberry pi GPIO Library
#include <lcd.h>				// raspberry pi LCD module header
#include <lirc/lirc_client.h>	// raspberry pi IR sensor hearder

// LCD module PIN number
#define LCD_RS	25
#define LCD_E	24
#define LCD_D4	23
#define LCD_D5	22
#define LCD_D6	21
#define LCD_D7	14

int client_len;
int client_sockfd;
char buf[256];
struct sockaddr_in clientaddr;

void downloadFromServer() {
		int nbyte;
		size_t filesize = 0, bufsize = 0;
		FILE *file = NULL;


		char* fileName = NULL;
		int fileNameSize = 0;

		recv(client_sockfd, &fileNameSize, sizeof(fileNameSize), 0);
		recv(client_sockfd, buf, fileNameSize, 0);

		printf("fileNameSize is %d\n", fileNameSize);
		printf("fileName is %s\n", buf);

		file = fopen(buf, "wb");

		ntohl(filesize);
		recv(client_sockfd, &filesize, sizeof(filesize), 0);
		printf("file size = [%d]\n", filesize);
		bufsize = 256;
		while(filesize != 0)
		{
				if(filesize < 256)
						bufsize = filesize;

				nbyte = recv(client_sockfd, buf, bufsize, 0);
				filesize = filesize -nbyte;

				fwrite(buf, sizeof(char), nbyte, file);

				nbyte = 0;

		}    
		fclose(file);
}

int main(int argc, char **argv)
{
		int lcd;
		struct lirc_config *config;
		int buttonTimer = millis();
		char *code;

		clientaddr.sin_family = AF_INET;
		clientaddr.sin_addr.s_addr = inet_addr("163.180.142.61");
		clientaddr.sin_port = htons(8002);
		client_len = sizeof(clientaddr);

		client_sockfd = socket(AF_INET, SOCK_STREAM, 0);

		if (connect (client_sockfd, (struct sockaddr *)&clientaddr,
								client_len) < 0)
		{
				perror("connect error :");
				exit(0);
		}

		if(wiringPiSetup() < 0) {
				perror("wiringPiSetup");
				exit(1);
		}

		// LCD initialize
		lcd = lcdInit(2, 16, 4, LCD_RS, LCD_E, LCD_D4,LCD_D5,LCD_D6,LCD_D7, 0, 0, 0, 0);
		if(lirc_init("lirc", 1) < 0) {
				perror("lirc");
				exit(1);
		}

		while(1) {
				// Read IR Signal
				if(lirc_readconfig(NULL, &config, NULL) == 0) {
						while(lirc_nextcode(&code) == 0) {
								if(code == NULL) continue; {
										// Time delay를 통한 Button bouncing problem 해결
										if(millis() - buttonTimer > 400) {
												// Input Previous button
												if(strstr(code, "KEY_PREVIOUS")) {
														printf("PREVIOUS");
														lcdPuts(lcd, "PREVIOS");	
														buttonTimer = millis();
												}
												// Input Next button
												else if(strstr(code, "KEY_NEXT")) {
														printf("NEXT");
														lcdPuts(lcd, "NEXT");	
														buttonTimer = millis();
												}

												// Input Play button
												else if(strstr(code, "KEY_PLAY")) {
														downloadFromServer();
														lcdPuts(lcd, "PLAY");	
														// system 함수를 통한 wav file 재생
														system("omxplayer -o local example.wav");
														buttonTimer = millis();
												}
												else {
														lcdPuts(lcd, "ERROR");	
														buttonTimer = millis();
												}
										}
								}

						}
				}
				else {
						// 대기상태, Hello USP 출력
						lcdPuts(lcd, "Hello USP");
				}
		}

		close(client_sockfd);
		return 0;
}

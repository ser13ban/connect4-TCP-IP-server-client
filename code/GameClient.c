#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <string.h>
#include <stdbool.h>

int translate_position(int pos);
void display_table(char table[8][15]);
void make_move(char table[8][15], int pos);


int main(int argc, char *argv[])
{
	char message[6]; //to display the message
	char table[8][15];
	int nr, rounds_counter = 0;
	int server, portNumber;
	socklen_t len;
	struct sockaddr_in servAdd;
	if (argc != 3)
	{
		printf("Call model:%s <IP> <Port#>\n", argv[0]);
		exit(0);
	}
	//creating and checking if socket is available
	if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "Cannot create socket\n");
		exit(1);
	}
	//setting domain to run over the internet
	servAdd.sin_family = AF_INET;
	//formating the Port Number
	sscanf(argv[2], "%d", &portNumber);
	//host to network conversion of multibyte integer
	servAdd.sin_port = htons((uint16_t)portNumber);
	//converts an address from presentation to network format
	if (inet_pton(AF_INET, argv[1], &servAdd.sin_addr) < 0)
	{
		fprintf(stderr, " inet_pton() has failed\n");
		exit(2);
	}
	// Establishing connection with TCP server
	if (connect(server, (struct sockaddr *)&servAdd, sizeof(servAdd)) < 0)
	{ //check if connection is created
		fprintf(stderr, "connect() has failed, exiting\n");
		exit(3);
	}

	while (1)
	{

		if (read(server, message, 5) < 0)
		{

			fprintf(stderr, "read() error\n");
			printf("ERROR: I can't read");
			close(server);
			exit(3);
		}

		if (strcmp(message, "govw") == 0)
		{
			int my_score, opponent_score;
			char rsp[10];
			int r = -1;
			printf("I won the game\n");

			if(read(server, &my_score, sizeof(int))<0){printf("ERROR: I can't read from player");exit(0);}
			if(read(server, &opponent_score, sizeof(int))<0){printf("ERROR: I can't read form player");exit(0);}
			printf("My score is: %d\nMy opponent score is: %d \n", my_score, opponent_score);
			while (r <= -1 || r >= 2)
			{
				printf("Do you want to replay? 1 for yes, 0 for no: ");
				scanf("%d", &r);
			}

			if(write(server, &r, sizeof(int))<0){printf("ERROR: I can't write to server");exit(0);}

			if (r == 0)
			{
				close(server);
				exit(0);
			}
		}
		else if (strcmp(message, "govl") == 0)
		{
			int my_score, opponent_score;
			char rsp[10];
			int r2;

			printf("I lost the game\n");

			if(read(server, &my_score, sizeof(int))<0){printf("ERROR: I can't read form player");exit(0);}
			if(read(server, &opponent_score, sizeof(int))<0){printf("ERROR: I can't read form player");exit(0);}
			printf("My score is: %d\nMy opponent score is: %d \n", my_score, opponent_score);
			while (r2 <= -1 || r2 >= 2)
			{
				printf("Do you want to replay? 1 for yes, 0 for no: ");
				scanf("%d", &r2);
			}

			if(write(server, &r2, sizeof(int))<0){printf("ERROR: I can't write to server");exit(0);}

			if (r2 == 0)
			{
				close(server);
				exit(0);
			}
		}
		else
		{
			int colum;
			system("clear");
			printf("It is your turn\n");
			if (read(server, table, 120) < 0)
			{
				printf("ERROR: I can't read the table from the server");
				exit(0);
			}
			display_table(table);
			//I want to get a valid position from the user
			int validPosition = 0;
			while (validPosition == 0)
			{
				char rsp[10];
				printf("Give me the colum: ");
				fgets(rsp, 10, stdin);
				colum = atoi(rsp);
				colum = translate_position(colum);
				fflush(stdout);
				if (table[0][colum] == ' ')
				{
					validPosition = 1;
				}
			}
			sleep(1);
			if(write(server, &colum, sizeof(int))<0){printf("ERROR: I can't write to server");exit(0);}
			//here is wher I should make the move and then display the table again to the plyaer
			make_move(table, colum);
			fflush(stdout);
		}
	}
}

int translate_position(int pos)
{
	return pos + pos - 1;
}
void display_table(char table[8][15])
{
	for (int i = 0; i <= 7; ++i)
	{
		for (int j = 0; j <= 14; ++j)
		{
			if (table[i][j] == '@')
			{
				printf("%c[1;32m%c ", 27, table[i][j]);
			} //green color for the first player
			else if (table[i][j] == '#')
			{
				printf("%c[1;34m%c ", 27, table[i][j]);
			} //blue color for the second player
			else if (table[i][j] >= 49 && table[i][j] <= 55)
			{
				printf("%c[1;31m%c ", 27, table[i][j]);
			} //red color for the numbers
			else
			{
				printf("%c[0;00m%c ", 27, table[i][j]);
			}
		}
		printf("\n");
	}
}
void make_move(char table[8][15], int pos)
{
	int i = 0;
	while (table[i][pos] == ' ' && i < 6)
	{
		system("clear");
		if (i == 0)
		{

			table[i][pos] = 'X';
			display_table(table);
		}
		else
		{

			table[i - 1][pos] = ' ';
			table[i][pos] = 'X';
			display_table(table);
		}
		++i;
	}
}
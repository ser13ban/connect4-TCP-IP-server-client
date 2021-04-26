#include <sys/shm.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdbool.h>

void servicePlayers(int player1, int player2, char table[8][15], int p1_score, int p2_score);				 //function to maintain the fork
void initialize_table(char table[8][15]);
void make_move(char table[8][15], int pos, int player);
bool check_gameOver(char table[8][15], int player);
void manageZombies(int signum);

int main(int argc, char *argv[])
{
	int sd, player1, player2, portNumber;
	struct sockaddr_in servAdd;
	char table[8][15];
	initialize_table(table);

	fprintf(stderr, "Server IP : ");
	//to display the ip address
	system("hostname -I"); //local IP adress
	fprintf(stderr, "*************************************\n");
	if (argc != 2)
	{
		printf("Call model: %s <Port #>\n", argv[0]);
		exit(0);
	}
	// Creating the TCP Socket
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		fprintf(stderr, "Cannot create socket\n");
		exit(1);
	}
	//Prepare the sockaddr_in structure
	servAdd.sin_family = AF_INET;					//setting domain to run over the internet
	servAdd.sin_addr.s_addr = htonl(INADDR_ANY);	//I wnat ot attach the socket to my local Ip adress
	sscanf(argv[1], "%d", &portNumber);				//formating the Port Number
	servAdd.sin_port = htons((uint16_t)portNumber); //host to network conversion of multibyte integer for short

	// Binding the IP address with Port Number
	bind(sd, (struct sockaddr *)&servAdd, sizeof(servAdd));
	// Put the server in passive mode and wait for server to accept the player
	listen(sd, 10);

	while (1)
	{
		// block until player 1 approach
		player1 = accept(sd, (struct sockaddr *)NULL, NULL);
		printf("Got Player\n");
		// block until player 2 approach
		player2 = accept(sd, (struct sockaddr *)NULL, NULL);
		printf("Got Player\n");
		//let the fork handle the game
		if (!fork())
		{
			servicePlayers(player1, player2, table, 0, 0);
			signal(SIGCHLD, manageZombies);
		}

		close(player1);
		close(player2);
	}
}
void manageZombies(int signum)
{
	wait(NULL);
}
void servicePlayers(int player1, int player2, char table[8][15], int p1_score, int p2_score)
{
	int colum, rp1, rp2;
	char resp[255];

	char *msg = "play";
	char *msg_win = "govw";	 //game over win
	char *msg_lost = "govl"; //gane over lost

	while (1)
	{

		if (check_gameOver(table, 1) == false)
		{
			p1_score++;

			if (write(player1, msg_win, strlen(msg) + 1) < 0)
			{
				printf("ERROR: I can't write to plyaer 1");
				exit(0);
			}
			if (write(player1, &p1_score, sizeof(int)) < 0)
			{
				printf("ERROR: I can't write to player 1");
				exit(0);
			}
			if (write(player1, &p2_score, sizeof(int)) < 0)
			{
				printf("ERROR: I can;t write to player 1");
				exit(0);
			}
			if (read(player1, &rp1, sizeof(int)) < 0)
			{
				printf("ERROR: I can't read from player 1");
				exit(0);
			}

			if (write(player2, msg_lost, strlen(msg) + 1) < 0)
			{
				printf("ERROR: I can' write to plyaer 2");
				exit(0);
			}
			if (write(player2, &p2_score, sizeof(int)) < 0)
			{
				printf("ERROR: I can't write to player 2");
				exit(0);
			}
			if (write(player2, &p1_score, sizeof(int)) < 0)
			{
				printf("ERROR: I can't write to player 2");
				exit(0);
			}
			if (read(player2, &rp2, sizeof(int)) < 0)
			{
				printf("ERROR: I can't read from player 2");
				exit(0);
			}

			if (rp1 == 1 && rp2 == 1)
			{
				initialize_table(table);
				servicePlayers(player1, player2, table, p1_score, p2_score);
			}
			else if (rp1 == 0 || rp2 == 0)
			{
				close(player1);
				close(player2);
				exit(0);
			}
		}
		else
		{
			if (write(player1, msg, strlen(msg) + 1) < 0)
			{
				printf("ERROR: I can't write to player 1");
				exit(0);
			}
			if (write(player1, table, 120) < 0)
			{
				printf("ERROR: I can't write to player 1");
				exit(0);
			}
			//read the collum fro the player1 (the client always send a good colum)
			if (read(player1, &colum, sizeof(int)) < 0)
			{
				printf("ERROR: I can't raed from plaer 1");
				exit(0);
			}
			//take that spot (make move);
			make_move(table, colum, 1);
		}

		sleep(1);
		if (check_gameOver(table, 2) == false)
		{
			p2_score++;

			if (write(player2, msg_win, strlen(msg) + 1) < 0)
			{
				printf("ERROR: I can't write to player 2");
				exit(0);
			}
			if (write(player2, &p2_score, sizeof(int)) < 0)
			{
				printf("ERROR: I can't write to player 2");
				exit(0);
			}
			if (write(player2, &p1_score, sizeof(int)) < 0)
			{
				printf("ERROR: I can' write to player 2");
				exit(0);
			}
			if (read(player2, &rp2, sizeof(int)) < 0)
			{
				printf("ERROR: I can't read to player 2");
				exit(0);
			}

			if (write(player1, msg_lost, strlen(msg) + 1) < 0)
			{
				printf("ERROR: I can't write to player 1");
				exit(0);
			}
			if (write(player1, &p1_score, sizeof(int)) < 0)
			{
				printf("ERROR: I can't write to player 1");
				exit(0);
			}
			if (write(player1, &p2_score, sizeof(int)) < 0)
			{
				printf("ERROR: I can't write to playe 1");
				exit(0);
			}
			if (read(player1, &rp1, sizeof(int)) < 0)
			{
				printf("ERROR: I can't read from player 1");
				exit(0);
			}
			if (rp1 == 1 && rp2 == 1)
			{
				initialize_table(table);
				servicePlayers(player1, player2, table, p1_score, p2_score);
			}
			else
			{
				close(player1);
				close(player2);
				exit(0);
			}
		}
		else
		{
			if (write(player2, msg, strlen(msg) + 1) < 0)
			{
				printf("ERROR: I can't write to plyar 2");
				exit(0);
			}
			if (write(player2, table, 120) < 0)
			{
				printf("ERROR: I can't write to plyar 2");
				exit(0);
			}
			//read teh collum fro the player1 (the client always send a good colum this is checked in the client side)
			if (read(player2, &colum, sizeof(int)) < 0)
			{
				printf("ERROR: I can't write to plyar 2");
				exit(0);
			}

			//take that spot (make move);
			make_move(table, colum, 2);
		}
	}
}

void initialize_table(char table[8][15])
{
	int i, j, aux = 1;
	for (i = 0; i <= 7; ++i)
	{
		for (j = 0; j <= 14; ++j)
		{
			table[i][j] = ' ';
		}
	}
	for (i = 0; i <= 5; ++i)
	{
		for (j = 0; j <= 14; j += 2)
		{
			table[i][j] = '|';
		}
	}
	for (j = 1; j <= 13; j += 2)
	{
		table[6][j] = aux + '0';
		aux++;
	}
}
void display_table(char table[8][15])
{
	for (int i = 0; i <= 7; ++i)
	{
		for (int j = 0; j <= 14; ++j)
		{
			printf("%c ", table[i][j]);
		}
		printf("\n");
	}
}
void make_move(char table[8][15], int pos, int player)
{
	int i = 0;
	while (table[i][pos] == ' ' && i < 6)
	{
		if (i == 0)
		{
			if (player == 1)
			{
				table[i][pos] = '@';
			}
			else
			{
				table[i][pos] = '#';
			}
		}
		else
		{
			if (player == 1)
			{
				table[i - 1][pos] = ' ';
				table[i][pos] = '@';
			}
			else
			{
				table[i - 1][pos] = ' ';
				table[i][pos] = '#';
			}
		}
		++i;
	}
}
bool check_gameOver(char table[8][15], int player)
{
	int i, j, k, count = 0, aux;
	char pl;
	if (player == 1)
	{
		pl = '@';
	}
	else
	{
		pl = '#';
	}
	//checking vertically
	for (j = 1; j <= 13; ++j)
	{
		for (i = 5; i >= 3; i--)
		{
			count = 1;
			while (i >= 0 && table[i][j] == pl && (table[i][j] == table[i - 1][j]))
			{
				count++;
				if (count == 4)
				{
					return false;
				}
				i--;
			}
		}
	}
	//checking horizontally:
	for (i = 5; i >= 0; i--)
	{
		for (j = 1; j <= 7; j += 2)
		{
			count = 1;
			while (j <= 13 && table[i][j] == pl && (table[i][j] == table[i][j + 2]))
			{
				count++;
				if (count == 4)
				{
					return false;
				}
				j += 2;
			}
		}
	}

	//chekcing diagonally
	//first direciton: left to right
	i = 5;
	j = 1;
	for (int k = 5; k >= 3; k--)
	{
		aux = k * 2 + 1;
		i = k;
		j = 1;
		count = 0;
		count = 0;
		while (i >= 0 && j <= aux && table[i][j] == pl)
		{
			count++;
			if (count == 4)
			{
				return false;
			}
			i--;
			j += 2;
		}
	}
	for (int k = 3; k <= 7; k = k + 2)
	{
		j = k;
		i = 5;
		count = 0;
		while (i >= 0 && j <= 13 && table[i][j] == pl)
		{
			count++;
			if (count == 4)
			{
				return false;
			}
			i--;
			j += 2;
		}
	}

	//second direction: from right to left
	aux = 5;
	for (int k = 3; k <= 7; k += 2)
	{
		i = aux;
		aux--;
		j = 13;
		count = 0;
		while (i >= 0 && j >= k && table[i][j] == pl)
		{
			count++;
			if (count == 4)
			{
				return false;
			}
			i--;
			j -= 2;
		}
	}
	aux = 0;
	for (int k = 11; k >= 7; k -= 2)
	{
		i = 5;
		j = k;
		count = 0;
		while (j >= 1 && i >= aux && table[i][j] == pl)
		{
			count++;
			if (count == 4)
			{
				return false;
			}
			i--;
			j -= 2;
		}
		aux++;
	}

	return true;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define RECV_BUFF_LEN 256

int main(int argc, char* argv[]) {
	/* Création d'une socket */
	int serverSocket;
	if (!~(serverSocket = socket(PF_INET, SOCK_STREAM, 0))) {
		perror("Client echo: socket error\n");
		return 1;
	}
	int socklen = sizeof(struct sockaddr_in);
	/* Attachement / nommage d'une socket */
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, socklen);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));
	/* Demande de connexion */
	if (!~connect(serverSocket, (struct sockaddr*) &serv_addr, socklen)) {
		perror("Client echo: connect error\n");
		return 3;
	}
	/* Échange de données */
	char* recv_buff = malloc(RECV_BUFF_LEN * sizeof(char));
	char* send_buff = malloc(5 * sizeof(char));
	send_buff[0] = 1;
	strcpy(&send_buff[1], "TIC@TAC");
	if (send(serverSocket, send_buff, (strlen(&send_buff[1]) + 2) * sizeof(char), 0) < 2) {
		perror("Client echo: send error\n");
		return 4;
	}
	do {
		if (recv(serverSocket, recv_buff, RECV_BUFF_LEN * sizeof(char), 0) < 2) {
			perror("Client echo: recv error\n");
			return 5;
		}
	} while (recv_buff[0] != 1);
	printf("%s\n", &recv_buff[1]);
	send_buff[0] = 3;
	int times = 10;
	while (times--) {
		sleep(1);
		if (send(serverSocket, send_buff, (strlen(&send_buff[1]) + 2) * sizeof(char), 0) < 2) {
			perror("Client echo: send error\n");
			return 4;
		}
		do {
			if (recv(serverSocket, recv_buff, RECV_BUFF_LEN * sizeof(char), 0) < 2) {
				perror("Client echo: recv error\n");
				return 5;
			}
		} while (recv_buff[0] != 3);
		printf("%s\n", &recv_buff[1]);
	}
	send_buff[0] = 2;
	send_buff[1] = '\0';
	if (send(serverSocket, send_buff, (strlen(&send_buff[1]) + 2) * sizeof(char), 0) < 2) {
		perror("Client echo: send error\n");
		return 4;
	}
	do {
		if (recv(serverSocket, recv_buff, RECV_BUFF_LEN * sizeof(char), 0) < 2) {
			perror("Client echo: recv error\n");
			return 5;
		}
	} while (recv_buff[0] != 2);
	printf("%s\n", &recv_buff[1]);
	free(send_buff);
	/* Fermeture d'un socket */
	close(serverSocket);
	free(recv_buff);
	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

const int SERV_PORT = 2222;
const int RECV_BUFF_LEN = 256;

int main(int argc, char* argv[]) {
	/* Création d'une socket */
	int serverSocket;
	if (!~(serverSocket = socket(PF_INET, SOCK_STREAM, 0))) {
		perror("Server echo: socket error\n");
		return 1;
	}
	int clilen = sizeof(struct sockaddr_in);
	/* Attachement / nommage d'une socket */
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(SERV_PORT);
	if (!~bind(serverSocket, (struct sockaddr*) &serv_addr, clilen)) {
		perror("Server echo: bind error\n");
		return 2;
	}
	/* Ouverture du service */
	if (!~listen(serverSocket, SOMAXCONN)) {
		perror("Server echo: listen error\n");
		return 3;
	}
	for (;;) {
		/* Création d'une socket de dialogue */
		int dialogSocket;
		struct sockaddr_in cli_addr;
		if (!~(dialogSocket = accept(serverSocket, (struct sockaddr*) &cli_addr, &clilen))) {
			perror("Server echo: accept error\n");
			return 4;
		}
		/* Échange de données */
		int id = fork();
		if(!~id) {
			perror("Server echo: fork error\n");
			return 5;
		} else if (!id) {
			char* recv_buff = malloc(RECV_BUFF_LEN * sizeof(char));
			char* send_buff = "TIC";
			close(serverSocket);
			do {
				printf("%s\n", recv_buff);
				sleep(1);
				if (!~send(dialogSocket, send_buff, strlen(send_buff) * sizeof(char), 0)) {
					perror("Server echo: send error\n");
					return 6;
				}
			} while (recv(dialogSocket, recv_buff, RECV_BUFF_LEN * sizeof(char), 0) > 0);
			free(recv_buff);
			break;
		} else {
			close(dialogSocket);
		}
	}
	/* Fermeture d'un socket */
	close(serverSocket);
	return 0;
}

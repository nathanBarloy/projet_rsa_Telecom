#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#define PORT 2222
#define LENGTH 140
#define SIGN_UP 0
#define SIGN_IN 1
#define SIGN_OUT 2
#define TWEET 3
#define FOLLOW_USER 4
#define FOLLOW_TAG 5
#define LIST_USERS 6
#define LIST_FOLLOWED_USERS 7
#define LIST_FOLLOWED_TAGS 8

int response(int socket, int type, char* message) {
	int send_size = (strlen(message) + 2) * sizeof(char);
	char* send_buff = malloc(send_size);
	send_buff[0] = type;
	strcpy(&send_buff[1], message);
	if (send(socket, send_buff, send_size, 0) >= 2) {
		send_size = 0;
	} else {
		send_size = 1;
	}
	free(send_buff);
	return send_size;
}

int sign_up(int socket, char* message) {
	return 0;
}

int sign_in(int socket, char* message) {
	return 0;
}

int sign_out(int socket, char* message) {
	return response(socket, 2, "") || 1;
}

int tweet(int socket, char* message) {
	printf("%s\n", message);
	return response(socket, 3, "TAC");
}

int follow_user(int socket, char* message) {
	return 0;
}

int follow_tag(int socket, char* message) {
	return 0;
}

int list_users(int socket, char* message) {
	return 0;
}

int list_followed_users(int socket, char* message) {
	return 0;
}

int list_followed_tags(int socket, char* message) {
	return 0;
}

int request(int socket) {
	int recv_size = (LENGTH + 2) * sizeof(char);
	char* recv_buff = malloc(recv_size);
	if ((recv_size = recv(socket, recv_buff, recv_size, 0)) >= 2) {
		recv_buff[recv_size] = '\0';
		int type = recv_buff[0];
		char* message = &recv_buff[1];
		switch (type) {
			case SIGN_UP: {
				recv_size = sign_up(socket, message);
				break;
			}
			case SIGN_IN: {
				recv_size = sign_in(socket, message);
				break;
			}
			case SIGN_OUT: {
				recv_size = sign_out(socket, message);
				break;
			}
			case TWEET: {
				recv_size = tweet(socket, message);
				break;
			}
			case FOLLOW_USER: {
				recv_size = follow_user(socket, message);
				break;
			}
			case FOLLOW_TAG: {
				recv_size = follow_tag(socket, message);
				break;
			}
			case LIST_USERS: {
				recv_size = list_users(socket, message);
				break;
			}
			case LIST_FOLLOWED_USERS: {
				recv_size = list_followed_users(socket, message);
				break;
			}
			case LIST_FOLLOWED_TAGS: {
				recv_size = list_followed_tags(socket, message);
				break;
			}
		}
	} else {
		recv_size = 0;
	}
	free(recv_buff);
	return recv_size;
}

int main(int argc, char* argv[]) {
	/* Création d'une socket */
	int server_socket;
	if (!~(server_socket = socket(PF_INET, SOCK_STREAM, 0))) {
		perror("Server echo: socket error\n");
		return 1;
	}
	socklen_t address_size = sizeof(struct sockaddr_in);
	/* Attachement / nommage d'une socket */
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(struct sockaddr_in));
	server_address.sin_family = PF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(PORT);
	if (!~bind(server_socket, (struct sockaddr*) &server_address, address_size)) {
		perror("Server echo: bind error\n");
		return 2;
	}
	/* Ouverture du service */
	if (!~listen(server_socket, SOMAXCONN)) {
		perror("Server echo: listen error\n");
		return 3;
	}

	int client_sockets[FD_SETSIZE];
	for (int i = 0; i < FD_SETSIZE; i++) {
		client_sockets[i] = -1;
	}
	int client_socket;
	struct sockaddr_in client_address;
	int socket_max = server_socket + 1;
	int socket_index;
	int socket_count;
	fd_set current_sockets;
	fd_set next_sockets;
	FD_ZERO(&current_sockets);
	FD_ZERO(&next_sockets);
	FD_SET(server_socket, &next_sockets);

	for (;;) {
		current_sockets = next_sockets;
		/* Création des sockets de dialogue */
		if (!~(socket_count = select(socket_max, &current_sockets, NULL, NULL, NULL))) {
			perror("Server echo: select error\n");
			return 4;
		}
		if (FD_ISSET(server_socket, &current_sockets)) {
			if (!~(client_socket = accept(server_socket, (struct sockaddr*) &client_address, &address_size))) {
				perror("Server echo: accept error\n");
				return 5;
			}
			socket_index = 0;
			while (socket_index < FD_SETSIZE && client_sockets[socket_index] > 0) {
				socket_index++;
			}
			if (socket_index == FD_SETSIZE) {
				exit(1);
			}
			printf("Connexion client (socket %d)\n", client_socket);
			client_sockets[socket_index] = client_socket;
			FD_SET(client_socket, &next_sockets);
			if (client_socket >= socket_max) {
				socket_max = client_socket + 1;
			}
			socket_count--;
		}
		/* Échange de données */
		socket_index = 0;
		while (socket_count > 0 && socket_index < FD_SETSIZE) {
			if ((client_socket = client_sockets[socket_index]) >= 0 && FD_ISSET(client_socket, &current_sockets)) {
				if (request(client_socket)) {
					printf("Déconnexion client (socket %d)\n", client_socket);
					close(client_socket);
					client_sockets[socket_index] = -1;
					FD_CLR(client_socket, &next_sockets);
				}
				socket_count--;
			}
			socket_index++;
		}
	}
}

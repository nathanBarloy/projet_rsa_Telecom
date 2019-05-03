#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#define PORT 2222
#define LENGTH 140
#define SIGN_UP 0
#define SIGN_IN 1
#define SIGN_OUT 2
#define TWEET 3
#define FOLLOW_USER 4
#define FOLLOW_TAG 5
#define LIST_FOLLOWED_USERS 6
#define LIST_FOLLOWED_TAGS 7
#define LIST_FOLLOWERS 8

int test_identifier(char* string) {
	for (int i = 0; string[i] != '\0'; i++) {
		if (
			(string[i] != '-' && string[i] < '0') ||
			(string[i] >= ':' && string[i] < 'A') ||
			(string[i] >= '[' && string[i] != '_' && string[i] < 'a') ||
			string[i] >= '{'
		) {
			return 1;
		}
	}
	return 0;
}

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

int sign_up(int sockets[], char* users[], int index, char* message) {
	int socket = sockets[index];
	if (users[index] != NULL) {
		return response(socket, SIGN_UP, "User already signed in");
	}
	if (message[0] == '\0') {
		return response(socket, SIGN_UP, "Neither user name nor password provided");
	}
	if (message[0] == '@') {
		return response(socket, SIGN_UP, "No user name provided");
	}
	char* password = strchr(message, '@');
	if (password == NULL) {
		return response(socket, SIGN_UP, "No password provided");
	}
	password[0] = '\0';
	password = &password[1];
	if (test_identifier(message)) {
		return response(socket, SIGN_UP, "Invalid user name");
	}
	if (test_identifier(password)) {
		return response(socket, SIGN_UP, "Invalid password");
	}
	char* home = getpwuid(getuid())->pw_dir;
	int home_length = strlen(home);
	int message_length = strlen(message);
	int path_length = home_length + 13 + message_length;
	char* path = malloc((path_length + 15) * sizeof(char));
	strcpy(path, home);
	strcpy(&path[home_length], "/.my-twitter/");
	mkdir(path, 0700);
	strcpy(&path[home_length + 13], message);
	if (!~mkdir(path, 0700)) {
		free(path);
		return response(socket, SIGN_UP, "User name not available");
	}
	path[path_length] = '/';
	strcpy(&path[path_length + 1], "password.txt");
	FILE* stream = fopen(path, "w");
	fprintf(stream, "%s\n", password);
	fclose(stream);
	strcpy(&path[path_length + 1], "users.txt");
	fclose(fopen(path, "w"));
	strcpy(&path[path_length + 1], "tags.txt");
	fclose(fopen(path, "w"));
	strcpy(&path[path_length + 1], "followers.txt");
	fclose(fopen(path, "w"));
	free(path);
	users[index] = malloc((message_length + 1) * sizeof(char));
	strcpy(users[index], message);
	return response(socket, SIGN_UP, "");
}

int sign_in(int sockets[], char* users[], int index, char* message) {
	int socket = sockets[index];
	if (users[index] != NULL) {
		return response(socket, SIGN_IN, "User already signed in");
	}
	if (message[0] == '\0') {
		return response(socket, SIGN_IN, "Neither user name nor password provided");
	}
	if (message[0] == '@') {
		return response(socket, SIGN_IN, "No user name provided");
	}
	char* password = strchr(message, '@');
	if (password == NULL) {
		return response(socket, SIGN_IN, "No password provided");
	}
	password[0] = '\0';
	password = &password[1];
	if (test_identifier(message)) {
		return response(socket, SIGN_IN, "Invalid user name");
	}
	if (test_identifier(password)) {
		return response(socket, SIGN_IN, "Invalid password");
	}
	char* home = getpwuid(getuid())->pw_dir;
	int home_length = strlen(home);
	int message_length = strlen(message);
	int path_length = home_length + 13 + message_length;
	char* path = malloc((path_length + 14) * sizeof(char));
	strcpy(path, home);
	strcpy(&path[home_length], "/.my-twitter/");
	strcpy(&path[home_length + 13], message);
	strcpy(&path[path_length], "/password.txt");
	FILE* stream = fopen(path, "r");
	free(path);
	if (stream == NULL) {
		return response(socket, SIGN_IN, "Incorrect user name");
	}
	char line[139];
	fscanf(stream, "%[^\n]\n", line);
	fclose(stream);
	if (strcmp(line, password)) {
		return response(socket, SIGN_IN, "Incorrect password");
	}
	for (int i = 0; i < FD_SETSIZE; i++) {
		char* user = users[i];
		if (user != NULL) {
			if (!strcmp(user, message)) {
				return response(socket, SIGN_IN, "User already signed in elsewhere");
			}
		}
	}
	users[index] = malloc((message_length + 1) * sizeof(char));
	strcpy(users[index], message);
	return response(socket, SIGN_IN, "");
}

int sign_out(int sockets[], char* users[], int index, char* message) {
	return response(sockets[index], SIGN_OUT, "") || 1;
}

int tweet(int sockets[], char* users[], int index, char* message) {
	if (users[index] == NULL) {
		return response(sockets[index], TWEET, "User not signed in");
	}
	printf("%s\n", message);
	return response(sockets[index], TWEET, "");
}

int follow_user(int sockets[], char* users[], int index, char* message) {
	int socket = sockets[index];
	char* user = users[index];
	if (users == NULL) {
		return response(socket, FOLLOW_USER, "User not signed in");
	}
	if (test_identifier(message)) {
		return response(socket, FOLLOW_USER, "Invalid user name");
	}
	if (!strcmp(message, user)) {
		return response(socket, FOLLOW_USER, "User is you");
	}
	char* home = getpwuid(getuid())->pw_dir;
	int home_length = strlen(home);
	int user_length = strlen(user);
	int path_length = home_length + 13 + user_length;
	char* path = malloc((path_length + 11) * sizeof(char));
	strcpy(path, home);
	strcpy(&path[home_length], "/.my-twitter/");
	strcpy(&path[home_length + 13], user);
	strcpy(&path[path_length], "/users.txt");
	FILE* stream = fopen(path, "a+");
	free(path);
	char line[139];
	for (;;) {
		if (fscanf(stream, "%[^\n]\n", line) == EOF) {
			break;
		}
		if (!strcmp(line, message)) {
			fclose(stream);
			return response(socket, FOLLOW_USER, "User already followed");
		}
	}
	fprintf(stream, "%s\n", message);
	fclose(stream);
	return response(socket, FOLLOW_USER, "");
}

int follow_tag(int sockets[], char* users[], int index, char* message) {
	if (users[index] == NULL) {
		return response(sockets[index], FOLLOW_TAG, "User not signed in");
	}
	return 0;
}

int list_followed_users(int sockets[], char* users[], int index, char* message) {
	if (users[index] == NULL) {
		return response(sockets[index], LIST_FOLLOWED_USERS, "User not signed in");
	}
	return 0;
}

int list_followed_tags(int sockets[], char* users[], int index, char* message) {
	if (users[index] == NULL) {
		return response(sockets[index], LIST_FOLLOWED_TAGS, "User not signed in");
	}
	return 0;
}

int list_followers(int sockets[], char* users[], int index, char* message) {
	if (users[index] == NULL) {
		return response(sockets[index], LIST_FOLLOWERS, "User not signed in");
	}
	return 0;
}

int request(int sockets[], char* users[], int index) {
	printf("%s\n", "HERE");
	int recv_size = (LENGTH + 2) * sizeof(char);
	char* recv_buff = malloc(recv_size);
	if ((recv_size = recv(sockets[index], recv_buff, recv_size, 0)) >= 2) {
		recv_buff[recv_size] = '\0';
		int type = recv_buff[0];
		char* message = &recv_buff[1];
		switch (type) {
			case SIGN_UP: {
				recv_size = sign_up(sockets, users, index, message);
				break;
			}
			case SIGN_IN: {
				recv_size = sign_in(sockets, users, index, message);
				break;
			}
			case SIGN_OUT: {
				recv_size = sign_out(sockets, users, index, message);
				break;
			}
			case TWEET: {
				recv_size = tweet(sockets, users, index, message);
				break;
			}
			case FOLLOW_USER: {
				recv_size = follow_user(sockets, users, index, message);
				break;
			}
			case FOLLOW_TAG: {
				recv_size = follow_tag(sockets, users, index, message);
				break;
			}
			case LIST_FOLLOWED_USERS: {
				recv_size = list_followed_users(sockets, users, index, message);
				break;
			}
			case LIST_FOLLOWED_TAGS: {
				recv_size = list_followed_tags(sockets, users, index, message);
				break;
			}
			case LIST_FOLLOWERS: {
				recv_size = list_followers(sockets, users, index, message);
				break;
			}
		}
	} else {
		recv_size = 1;
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
	char* client_users[FD_SETSIZE];
	for (int i = 0; i < FD_SETSIZE; i++) {
		client_sockets[i] = -1;
		client_users[i] = NULL;
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
			socket_index = -1;
			while (++socket_index < FD_SETSIZE && ~client_sockets[socket_index]);
			if (socket_index == FD_SETSIZE) {
				close(client_socket);
			}
			client_sockets[socket_index] = client_socket;
			FD_SET(client_socket, &next_sockets);
			if (client_socket >= socket_max) {
				socket_max = client_socket + 1;
			}
			socket_count--;
			printf("Connexion client (socket %d)\n", client_socket);
		}
		/* Échange de données */
		socket_index = 0;
		while (socket_count > 0 && socket_index < FD_SETSIZE) {
			if (~(client_socket = client_sockets[socket_index]) && FD_ISSET(client_socket, &current_sockets)) {
				if (request(client_sockets, client_users, socket_index)) {
					printf("Déconnexion client (socket %d)\n", client_socket);
					client_sockets[socket_index] = -1;
					if (client_users[socket_index] != NULL) {
						free(client_users[socket_index]);
						client_users[socket_index] = NULL;
					}
					if (socket_index == socket_max) {
						while (--socket_max > server_socket + 1 && !~client_sockets[socket_max]);
					}
					FD_CLR(client_socket, &next_sockets);
					close(client_socket);
				}
				socket_count--;
			}
			socket_index++;
		}
	}
	/* Fermeture d'une socket */
	close(server_socket);
	return 0;
}

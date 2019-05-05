#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

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
#define SHARE 9

static char* path;
static int base_length;
static FILE* stream;
static int server_socket;
static int client_socket;
static int client_sockets[FD_SETSIZE];
static char* client_users[FD_SETSIZE];
static char recv_buff[LENGTH + 2];
static char send_buff[LENGTH + 2];

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

int response(int client_socket, char type, char* message) {
	int send_size = (strlen(message) + 2) * sizeof(char);
	send_buff[0] = type;
	strcpy(&send_buff[1], message);
	if (send(client_socket, send_buff, send_size, 0) >= 2) {
		send_size = 0;
	} else {
		send_size = 1;
	}
	return send_size;
}

int sign_up(int index, char* message) {
	int client_socket = client_sockets[index];
	if (client_users[index] != NULL) {
		return response(client_socket, SIGN_UP, "User already signed in");
	}
	if (message[0] == '\0') {
		return response(client_socket, SIGN_UP, "Neither user name nor password provided");
	}
	if (message[0] == '@') {
		return response(client_socket, SIGN_UP, "No user name provided");
	}
	char* password = strchr(message, '@');
	if (password == NULL) {
		return response(client_socket, SIGN_UP, "No password provided");
	}
	password[0] = '\0';
	password = &password[1];
	if (test_identifier(message)) {
		return response(client_socket, SIGN_UP, "Invalid user name");
	}
	if (test_identifier(password)) {
		return response(client_socket, SIGN_UP, "Invalid password");
	}
	int message_length = strlen(message);
	int user_length = base_length + message_length;
	strcpy(&path[base_length], message);
	if (!~mkdir(path, 0700)) {
		return response(client_socket, SIGN_UP, "User name not available");
	}
	strcpy(&path[user_length], "/password.txt");
	stream = fopen(path, "w");
	fprintf(stream, "%s\n", password);
	fclose(stream), stream = NULL;
	strcpy(&path[user_length], "/users.txt");
	fclose(fopen(path, "w"));
	strcpy(&path[user_length], "/tags.txt");
	fclose(fopen(path, "w"));
	strcpy(&path[user_length], "/followers.txt");
	fclose(fopen(path, "w"));
	client_users[index] = malloc((message_length + 1) * sizeof(char));
	strcpy(client_users[index], message);
	return response(client_socket, SIGN_UP, "");
}

int sign_in(int index, char* message) {
	int client_socket = client_sockets[index];
	if (client_users[index] != NULL) {
		return response(client_socket, SIGN_IN, "User already signed in");
	}
	if (message[0] == '\0') {
		return response(client_socket, SIGN_IN, "Neither user name nor password provided");
	}
	if (message[0] == '@') {
		return response(client_socket, SIGN_IN, "No user name provided");
	}
	char* password = strchr(message, '@');
	if (password == NULL) {
		return response(client_socket, SIGN_IN, "No password provided");
	}
	password[0] = '\0';
	password = &password[1];
	if (test_identifier(message)) {
		return response(client_socket, SIGN_IN, "Invalid user name");
	}
	if (test_identifier(password)) {
		return response(client_socket, SIGN_IN, "Invalid password");
	}
	int message_length = strlen(message);
	int user_length = base_length + message_length;
	strcpy(&path[base_length], message);
	strcpy(&path[user_length], "/password.txt");
	stream = fopen(path, "r");
	if (stream == NULL) {
		return response(client_socket, SIGN_IN, "Incorrect user name");
	}
	char line[LENGTH + 1];
	fscanf(stream, "%[^\n]\n", line);
	fclose(stream), stream = NULL;
	if (strcmp(line, password)) {
		return response(client_socket, SIGN_IN, "Incorrect password");
	}
	for (int i = 0; i < FD_SETSIZE; i++) {
		char* client_user = client_users[i];
		if (client_user != NULL) {
			if (!strcmp(client_user, message)) {
				return response(client_socket, SIGN_IN, "User already signed in elsewhere");
			}
		}
	}
	client_users[index] = malloc((message_length + 1) * sizeof(char));
	strcpy(client_users[index], message);
	return response(client_socket, SIGN_IN, "");
}

int sign_out(int index, char* message) {
	return response(client_sockets[index], SIGN_OUT, "") || 1;
}

int tweet(int index, char* message) {
	int client_socket = client_sockets[index];
	char* client_user = client_users[index];
	if (client_user == NULL) {
		return response(client_socket, TWEET, "User not signed in");
	}
	fd_set shares;
	FD_ZERO(&shares);
	int user_length = base_length + strlen(client_user);
	strcpy(&path[base_length], client_user);
	strcpy(&path[user_length], "/followers.txt");
	stream = fopen(path, "r");
	char line[LENGTH + 1];
	while (fscanf(stream, "%[^\n]\n", line) != EOF) {
		for (int i = 0; i < FD_SETSIZE; i++) {
			if (i != index) {
				char* client_user = client_users[i];
				if (client_user != NULL && !strcmp(client_user, line)) {
					int client_socket = client_sockets[i];
					FD_SET(client_socket, &shares);
					response(client_socket, SHARE, message);
				}
			}
		}
	}
	fclose(stream), stream = NULL;
	for (int i = 0; message[i] != '\0'; i++) {
		if (message[i] == '#') {
			int j = ++i;
			while (
				message[j] == '-' ||
				(message[j] >= '0' && message[j] < ':') ||
				(message[j] >= 'A' && message[j] < '[') ||
				message[j] == '_' ||
				(message[j] >= 'a' && message[j] < '{')
			) {
				j++;
			}
			if (j != i) {
				char tag[LENGTH + 1];
				strncpy(tag, &message[i], (j - i) * sizeof(char));
				tag[j - i] = '\0';
				for (int k = 0; k < FD_SETSIZE; k++) {
					if (k != index) {
						char* client_user = client_users[k];
						if (client_user != NULL) {
							int client_socket = client_sockets[k];
							if (!FD_ISSET(client_socket, &shares)) {
								user_length = base_length + strlen(client_user);
								strcpy(&path[base_length], client_user);
								strcpy(&path[user_length], "/tags.txt");
								stream = fopen(path, "r");
								while (fscanf(stream, "%[^\n]\n", line) != EOF) {
									if (!strcmp(line, tag)) {
										FD_SET(client_socket, &shares);
										response(client_socket, SHARE, message);
										break;
									}
								}
								fclose(stream), stream = NULL;
							}
						}
					}
				}
				i = j;
			}
		}
	}
	return response(client_sockets[index], TWEET, "");
}

int follow_user(int index, char* message) {
	int client_socket = client_sockets[index];
	char* client_user = client_users[index];
	if (client_user == NULL) {
		return response(client_socket, FOLLOW_USER, "User not signed in");
	}
	if (test_identifier(message)) {
		return response(client_socket, FOLLOW_USER, "Invalid user name");
	}
	if (!strcmp(message, client_user)) {
		return response(client_socket, FOLLOW_USER, "User is you");
	}
	int user_length = base_length + strlen(message);
	strcpy(&path[base_length], message);
	strcpy(&path[user_length], "/password.txt");
	stream = fopen(path, "r");
	if (stream == NULL) {
		return response(client_socket, FOLLOW_USER, "Incorrect user name");
	}
	fclose(stream), stream = NULL;
	user_length = base_length + strlen(client_user);
	strcpy(&path[base_length], client_user);
	strcpy(&path[user_length], "/users.txt");
	stream = fopen(path, "a+");
	char line[LENGTH + 1];
	while (fscanf(stream, "%[^\n]\n", line) != EOF) {
		if (!strcmp(line, message)) {
			fclose(stream), stream = NULL;
			return response(client_socket, FOLLOW_USER, "User already followed");
		}
	}
	fprintf(stream, "%s\n", message);
	fclose(stream), stream = NULL;
	user_length = base_length + strlen(message);
	strcpy(&path[base_length], message);
	strcpy(&path[user_length], "/followers.txt");
	stream = fopen(path, "a");
	fprintf(stream, "%s\n", client_user);
	fclose(stream), stream = NULL;
	return response(client_socket, FOLLOW_USER, "");
}

int follow_tag(int index, char* message) {
	int client_socket = client_sockets[index];
	char* client_user = client_users[index];
	if (client_user == NULL) {
		return response(client_socket, FOLLOW_TAG, "User not signed in");
	}
	if (test_identifier(message)) {
		return response(client_socket, FOLLOW_TAG, "Invalid tag");
	}
	int user_length = base_length + strlen(client_user);
	strcpy(&path[base_length], client_user);
	strcpy(&path[user_length], "/tags.txt");
	stream = fopen(path, "a+");
	char line[LENGTH + 1];
	while (fscanf(stream, "%[^\n]\n", line) != EOF) {
		if (!strcmp(line, message)) {
			fclose(stream), stream = NULL;
			return response(client_socket, FOLLOW_TAG, "Tag already followed");
		}
	}
	fprintf(stream, "%s\n", message);
	fclose(stream), stream = NULL;
	return response(client_socket, FOLLOW_TAG, "");
}

int list_followed_users(int index, char* message) {
	int client_socket = client_sockets[index];
	char* client_user = client_users[index];
	if (client_user == NULL || message[0] == '\0' || message[1] != '\0') {
		return response(client_socket, LIST_FOLLOWED_USERS, "");
	}
	int user_length = base_length + strlen(client_user);
	strcpy(&path[base_length], client_user);
	strcpy(&path[user_length], "/users.txt");
	stream = fopen(path, "r");
	char count = message[0];
	int size = 0;
	char line[LENGTH * 2 + 2];
	while (fscanf(stream, "%[^\n]\n", &line[size]) != EOF) {
		int length = strlen(&line[size]);
		if (size + length <= LENGTH) {
			size += length;
			line[size++] = ',';
		} else if (count - 1) {
			count--;
			for (int i = 0; i < length; i++) {
				line[i] = line[size + i];
			}
			size = length;
			line[size++] = ',';
		} else {
			break;
		}
	}
	fclose(stream), stream = NULL;
	if (count - 1) {
		line[0] = '\0';
	} else {
		line[size ? size - 1 : 0] = '\0';
	}
	return response(client_socket, LIST_FOLLOWED_USERS, line);
}

int list_followed_tags(int index, char* message) {
	int client_socket = client_sockets[index];
	char* client_user = client_users[index];
	if (client_user == NULL || message[0] == '\0' || message[1] != '\0') {
		return response(client_socket, LIST_FOLLOWED_TAGS, "");
	}
	int user_length = base_length + strlen(client_user);
	strcpy(&path[base_length], client_user);
	strcpy(&path[user_length], "/tags.txt");
	stream = fopen(path, "r");
	char count = message[0];
	int size = 0;
	char line[(LENGTH * 1) * 2];
	while (fscanf(stream, "%[^\n]\n", &line[size]) != EOF) {
		int length = strlen(&line[size]);
		if (size + length <= LENGTH) {
			size += length;
			line[size++] = ',';
		} else if (count - 1) {
			count--;
			for (int i = 0; i < length; i++) {
				line[i] = line[size + i];
			}
			size = length;
			line[size++] = ',';
		} else {
			break;
		}
	}
	fclose(stream), stream = NULL;
	if (count - 1) {
		line[0] = '\0';
	} else {
		line[size ? size - 1 : 0] = '\0';
	}
	return response(client_socket, LIST_FOLLOWED_TAGS, line);
}

int list_followers(int index, char* message) {
	int client_socket = client_sockets[index];
	char* client_user = client_users[index];
	if (client_user == NULL || message[0] == '\0' || message[1] != '\0') {
		return response(client_socket, LIST_FOLLOWERS, "");
	}
	int user_length = base_length + strlen(client_user);
	strcpy(&path[base_length], client_user);
	strcpy(&path[user_length], "/followers.txt");
	stream = fopen(path, "r");
	char count = message[0];
	int size = 0;
	char line[(LENGTH * 1) * 2];
	while (fscanf(stream, "%[^\n]\n", &line[size]) != EOF) {
		int length = strlen(&line[size]);
		if (size + length <= LENGTH) {
			size += length;
			line[size++] = ',';
		} else if (count - 1) {
			count--;
			for (int i = 0; i < length; i++) {
				line[i] = line[size + i];
			}
			size = length;
			line[size++] = ',';
		} else {
			break;
		}
	}
	fclose(stream), stream = NULL;
	if (count - 1) {
		line[0] = '\0';
	} else {
		line[size ? size - 1 : 0] = '\0';
	}
	return response(client_socket, LIST_FOLLOWERS, line);
}

int request(int index) {
	int recv_size = (LENGTH + 2) * sizeof(char);
	if ((recv_size = recv(client_sockets[index], recv_buff, recv_size, 0)) >= 2) {
		recv_buff[recv_size] = '\0';
		char type = recv_buff[0];
		char* message = &recv_buff[1];
		switch (type) {
			case SIGN_UP: {
				recv_size = sign_up(index, message);
				break;
			}
			case SIGN_IN: {
				recv_size = sign_in(index, message);
				break;
			}
			case SIGN_OUT: {
				recv_size = sign_out(index, message);
				break;
			}
			case TWEET: {
				recv_size = tweet(index, message);
				break;
			}
			case FOLLOW_USER: {
				recv_size = follow_user(index, message);
				break;
			}
			case FOLLOW_TAG: {
				recv_size = follow_tag(index, message);
				break;
			}
			case LIST_FOLLOWED_USERS: {
				recv_size = list_followed_users(index, message);
				break;
			}
			case LIST_FOLLOWED_TAGS: {
				recv_size = list_followed_tags(index, message);
				break;
			}
			case LIST_FOLLOWERS: {
				recv_size = list_followers(index, message);
				break;
			}
			default: {
				recv_size = 0;
			}
		}
	} else {
		recv_size = 1;
	}
	return recv_size;
}

void stop(int signal) {
	if (server_socket != -1) {
		for (int i = 0; i < FD_SETSIZE; i++) {
			char* client_user = client_users[i];
			if (client_user != NULL) {
				free(client_user), client_user = NULL;
			}
			int current_client_socket = client_sockets[i];
			if (current_client_socket != -1) {
				response(current_client_socket, SIGN_OUT, "");
				printf("Client %d disconnected\n", current_client_socket - server_socket - 1);
				if (current_client_socket == client_socket) {
					client_socket = -1;
				}
				close(current_client_socket), client_sockets[i] = -1;
			}
		}
		if (client_socket != -1) {
			response(client_socket, SIGN_OUT, "");
			printf("Client %d disconnected\n", client_socket - server_socket - 1);
			close(client_socket), client_socket = -1;
		}
		close(server_socket), server_socket = -1;
	}
	if (stream != NULL) {
		fclose(stream), stream = NULL;
	}
	if (path != NULL) {
		free(path), path = NULL;
	}
	exit(signal + 128);
}

int main(int argc, char* argv[]) {
	path = NULL;
	base_length = -1;
	stream = NULL;
	server_socket = -1;
	client_socket = -1;
	for (int i = 0; i < FD_SETSIZE; i++) {
		client_sockets[i] = -1;
		client_users[i] = NULL;
	}
	struct sigaction new;
	struct sigaction old;
	memset(&new, 0, sizeof(struct sigaction));
	new.sa_handler = &stop;
	sigaction(SIGINT, &new, &old);
	char* home = getpwuid(getuid())->pw_dir;
	int home_length = strlen(home);
	path = malloc((LENGTH + 28) * sizeof(char));
	strcpy(path, home);
	strcpy(&path[home_length], "/.my-twitter/");
	mkdir(path, 0700);
	base_length = home_length + 13;
	if (!~(server_socket = socket(AF_INET6, SOCK_STREAM, 0))) {
		perror("`socket` error");
		stop(-127);
	}
	socklen_t address_size = sizeof(struct sockaddr_in6);
	struct sockaddr_in6 server_address;
	memset(&server_address, 0, address_size);
	server_address.sin6_family = AF_INET6;
	server_address.sin6_port = htons(argc > 1 ? atoi(argv[1]) : 2222);
	if (!~bind(server_socket, (struct sockaddr*) &server_address, address_size)) {
		perror("`bind` error");
		stop(-126);
	}
	if (!~listen(server_socket, SOMAXCONN)) {
		perror("`listen` error");
		stop(-125);
	}
	struct sockaddr_in6 client_address;
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
		if (!~(socket_count = select(socket_max, &current_sockets, NULL, NULL, NULL))) {
			perror("`select` error");
			stop(-124);
		}
		if (FD_ISSET(server_socket, &current_sockets)) {
			if (!~(client_socket = accept(server_socket, (struct sockaddr*) &client_address, &address_size))) {
				perror("`accept` error");
				stop(-123);
			}
			socket_index = -1;
			while (++socket_index < FD_SETSIZE && ~client_sockets[socket_index]);
			if (socket_index == FD_SETSIZE) {
				close(client_socket), client_socket = -1;
			}
			client_sockets[socket_index] = client_socket;
			FD_SET(client_socket, &next_sockets);
			if (client_socket >= socket_max) {
				socket_max = client_socket + 1;
			}
			socket_count--;
			printf("Client %d connected\n", client_socket - server_socket - 1);
		}
		socket_index = 0;
		client_socket = -1;
		while (socket_count > 0 && socket_index < FD_SETSIZE) {
			if (~(client_socket = client_sockets[socket_index]) && FD_ISSET(client_socket, &current_sockets)) {
				if (request(socket_index)) {
					printf("Client %d disconnected\n", client_socket - server_socket - 1);
					client_sockets[socket_index] = -1;
					if (client_users[socket_index] != NULL) {
						free(client_users[socket_index]), client_users[socket_index] = NULL;
					}
					if (socket_index == socket_max) {
						while (--socket_max > server_socket + 1 && !~client_sockets[socket_max]);
					}
					FD_CLR(client_socket, &next_sockets);
					close(client_socket), client_socket = -1;
				}
				socket_count--;
			}
			socket_index++;
		}
	}
	return 0;
}

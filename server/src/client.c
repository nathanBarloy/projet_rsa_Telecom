#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

const int RECV_BUFF_LEN = 256;

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
    char* send_buff = "TAC";
     while (recv(serverSocket, recv_buff, RECV_BUFF_LEN * sizeof(char), 0) > 0) {
        printf("%s\n", recv_buff);
        sleep(1);
        if (!~send(serverSocket, send_buff, strlen(send_buff) * sizeof(char), 0)) {
            perror("Client echo: send error\n");
            return 4;
        }
    }
    /* Fermeture d'un socket */
    close(serverSocket);
    free(recv_buff);
    return 0;
}

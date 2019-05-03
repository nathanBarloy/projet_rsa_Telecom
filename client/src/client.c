#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <termios.h>

#define MAXLINE 140

int serverSocket,servlen,n,retread;
struct sockaddr_in serv_addr;
char bufSend[MAXLINE];
char bufRecv[MAXLINE];
struct hostent *hp;


void viderBuffer()
{
    int c = 0;
    while (c != '\n' && c != EOF)
    {
        c = getchar();
    }
}

int affInfo(char *buff) {
    printf("type : %d\tcontenu : %s\n",(int)buff[0], &buff[1]);
}

void afficherMascotte() {

	char *turkey = "\n -----------------------------------------\n\
/ Bienvenue sur lca contrefaçon Twitter ! \\\n\
| Une application au top de la            |\n\
\\ technologie !                           /\n\
 -----------------------------------------\n\
  \\                                  ,+*^^*+___+++_\n\
   \\                           ,*^^^^              )\n\
    \\                       _+*                     ^**+_\n\
     \\                   +^       _ _++*+_+++_,         )\n\
              _+^^*+_    (     ,+*^ ^          \\+_        )\n\
             {       )  (    ,(    ,_+--+--,      ^)      ^\\\n\
            { (@)    } f   ,(  ,+-^ __*_*_  ^^\\_   ^\\       )\n\
           {:;-/    (_+*-+^^^^^+*+*<_ _++_)_    )    )      /\n\
          ( /  (    (        ,___    ^*+_+* )   <    <      \\\n\
           U _/     )    *--<  ) ^\\-----++__)   )    )       )\n\
            (      )  _(^)^^))  )  )\\^^^^^))^*+/    /       /\n\
          (      /  (_))_^)) )  )  ))^^^^^))^^^)__/     +^^\n\
         (     ,/    (^))^))  )  ) ))^^^^^^^))^^)       _)\n\
          *+__+*       (_))^)  ) ) ))^^^^^^))^^^^^)____*^\n\
          \\             \\_)^)_)) ))^^^^^^^^^^))^^^^)\n\
           (_             ^\\__^^^^^^^^^^^^))^^^^^^^)\n\
             ^\\___            ^\\__^^^^^^))^^^^^^^^)\\\n\
                  ^^^^^\\uuu/^^\\uuu/^^^^\\^\\^\\^\\^\\^\\^\\^\\\n\
                     ___) >____) >___   ^\\_\\_\\_\\_\\_\\_\\)\n\
                    ^^^//\\_^^//\\_^       ^(\\_\\_\\_\\)\n\
                      ^^^ ^^ ^^^ ^\n";




    printf("%s\n",turkey);
}



int envoyer(int type, char* message) {
	int send_size = (strlen(message) + 2) * sizeof(char);
	char* send_buff = malloc(send_size);
	send_buff[0] = type;
	strcpy(&send_buff[1], message);
	if (send(serverSocket, send_buff, send_size, 0) >= 2) {
		send_size = 0;
	} else {
		send_size = 1;
	}
	free(send_buff);
	return send_size;
}

int quitter() {
	envoyer(2,"");
	close(serverSocket);
	exit(0);
}

int menuConnecte(char *addIp, int port) {
  	char answer;
  	int cont = 0;
  	printf("\nQue voulez vous faire ?\n");
  	while (!cont) {
    	printf("t -> twitter\nu -> demander la liste des utilisateurs suivis\nv -> demander la liste des utilisateurs qui vous suivent\nh -> demander la liste des thématiques suivies\nd -> déconnexion\nq -> quitter l'application\nVotre choix : ");
    	answer = getchar();
    	if (answer && (answer=='t' || answer=='u' || answer=='v' || answer=='h' || answer=='d' || answer=='q')) {
     		cont = 1;
    	} else {
      		printf("\nVotre entrée n'est pas correcte. Réessayez.\n");
    	}
    		viderBuffer();
  	}
  	switch (answer) {
    	case 't' :
      		break;
    	case 'u' :
      		break;
    	case 'v' :
      		break;
    	case 'h' :
      		break;
    	case 'd' :
      		break;
    	case 'q' :
      		quitter();
      		break;
  	}

  	return 0;
}

void creer_compte() {
	char nomUtil[141];
	char mdp[141];
	char confmdp[141];
	char *toSend;
	int Cbon=0;
	int recv_size;

	printf("\n\n");
	while(!Cbon) {
		printf("Votre nom d'utilisateur :");
    	scanf("%140s",nomUtil);
    	printf("Votre mot de passe :");
    	scanf("%140s",mdp);
    	printf("Confirmation du mot de passe :");
    	scanf("%140s",confmdp);

    	if (strcmp(mdp,confmdp)==0 && nomUtil!=NULL && nomUtil!="") { //les données sont conformes
      		Cbon = 1;
      		toSend = malloc(sizeof(char)*(strlen(nomUtil)+strlen(mdp)+2));

      		strcpy(toSend,nomUtil);
      		strcat(toSend,"@");
      		strcat(toSend,mdp);

      		envoyer(0,toSend);

      		if ((recv_size = recv(serverSocket, bufRecv, MAXLINE, 0)) >= 2) {
      			bufRecv[recv_size] = '\0';
				int type = bufRecv[0];
				char* message = &bufRecv[1];
				affInfo(bufRecv);
      		}

      		menuPrincipal();
    	} else {
      		printf("\nVos données sont non conformes\n");
    	}
  	}
  	viderBuffer();

}


int connexion(char *addIp, int port) {

	//on remplis la structure serv_addre avec l'adresse du serveur
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	int portno = port;
    serv_addr.sin_port = htons(portno);
	hp=(struct hostent*)gethostbyname(addIp);
	if(hp==NULL){
		fprintf(stderr,"%s : %d ,p, trouve dans in /etc/hosts ou dans le DNS\n",addIp,port);
		exit(0);
	}
	serv_addr.sin_addr = *((struct in_addr *)(hp->h_addr));

	//on crée la socket
	if((serverSocket = socket(AF_INET,SOCK_STREAM,0))<0){
		perror("erreur socket");
		exit(1);
	}
	
	//on connecte la socket
	if(connect(serverSocket,(struct sockaddr *) &serv_addr,sizeof(serv_addr))<0){
		perror("erreur connect");
		exit(1);
	}
}


int menuPrincipal() {
	  /* lance le menu principal de l'appli
  */
	char answer;
	char name[MAXLINE];
  	char password[MAXLINE];
  	int cont = 0;



  	printf("\nQue voulez vous faire ?\n");
  	while (!cont) {
    	printf("c -> connexion\nn -> créer un nouveau compte\nq -> quitter l'application\nVotre choix : ");
    	answer = getchar();
    	if (answer && (answer=='c' || answer=='n' || answer=='q')) {
      		cont = 1;
    	} else {
      		printf("\nVotre entrée n'est pas correcte. Réessayez.\n");
    	}
    	viderBuffer();
	}
	switch (answer) {
    	case 'c' :
      		printf("\nNom d'utilisateur : ");
      		scanf("%s", name);
      		printf("Mot de passe : ");
	      	system("stty -echo");
	      	scanf("%s", password);
	      	system("stty echo");
	      	printf("\n%s\n%s\n",name,password);
	      	break;
    	case 'n' :
	      	creer_compte();
	      	break;
	    case 'q' :
	      	quitter();
	      	break;
  	}

  	return 0;
  
}


int main(int argc, char **argv){
	char *addIp = "127.0.0.1";
	int port = 2222;

	switch (argc) {
    	case 1 :
      		break;
    	case 2 :
      		addIp = argv[1];
      		break;
    	case 3 :
      		addIp = argv[1];
      		port = atoi(argv[2]);
      		break;
    	default :
      		printf("Trop d'arguments\n");
      		exit(1);
	}

	connexion(addIp, port);

	afficherMascotte();
	menuPrincipal();

	quitter();
	return 0;
}
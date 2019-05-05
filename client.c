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
#include <signal.h>

#define MAXLINE 140

//TODO afficher des listes et faire le suivi de thematiques et des utilisateurs


int serverSocket,servlen,n,retread;
struct sockaddr_in6 serv_addr;
char bufSend[MAXLINE];
char bufRecv[MAXLINE];
struct hostent *hp;




int compter_thematiques(char *texte) {
  int nbThematique = 0;
  int dansUnTheme=0;
  int tailleTheme=0;
  int i = -1;
  char c;

  do {

    i++;
      c = texte[i];

    switch(c) {

      case '#' :
        nbThematique++;
          dansUnTheme = 1;
          tailleTheme = 0;
          break;

        case ' ' :

        case '\0' :
          if (dansUnTheme) {
            dansUnTheme = 0;
            //printf("debut : %d et taille : %d\n", debutTheme,tailleTheme);
            if (tailleTheme==0) {
              nbThematique--;
            }
          }
          break;

        default :
          if (dansUnTheme) {
            tailleTheme++;
          }
          break;

      }


    } while (c!='\0');

    return nbThematique;
}

char **recherche_thematiques(char *texte) {
  /*cette fonction sert à extraire les thematiques d'un tweet*/

  int nbThematique = compter_thematiques(texte);

  char **res = malloc(sizeof(char*)*nbThematique);
  int dansUnTheme=0;
  int tailleTheme=0;
  int debutTheme=0;
  int i = -1;
  char c;
  char *newTheme;
  int numTheme = 0;

  do {

    i++;
      c = texte[i];

    switch(c) {

      case '#' :
        if (dansUnTheme) {
          dansUnTheme = 0;
            //printf("debut : %d et taille : %d\n", debutTheme,tailleTheme);
            if (tailleTheme==0) {
              numTheme--;
            } else {
              newTheme = malloc(sizeof(char)*(tailleTheme+1));
              strncpy(newTheme,texte+debutTheme, tailleTheme);
              //printf("%s\n", newTheme);
              res[numTheme-1] = newTheme;
            }
        }
        numTheme++;
          dansUnTheme = 1;
          debutTheme = i+1;
          tailleTheme = 0;
          break;

        case ' ' :

        case '\0' :
          if (dansUnTheme) {
            dansUnTheme = 0;
            //printf("debut : %d et taille : %d\n", debutTheme,tailleTheme);
            if (tailleTheme==0) {
              numTheme--;
            } else {
              newTheme = malloc(sizeof(char)*(tailleTheme+1));
              strncpy(newTheme,texte+debutTheme, tailleTheme);
              //printf("%s\n", newTheme);
              res[numTheme-1] = newTheme;
            }
          }
          break;

        default :
          if (dansUnTheme) {
            tailleTheme++;
          }
          break;

      }


    } while (c!='\0');
    return res;
}




void viderBuffer() {
    int c = 0;
    while (c != '\n' && c != EOF)
    {
        c = getchar();
    }
}

void affInfo(char *buff) {
    printf("type : %d\tcontenu : %s\n",(int)buff[0], &buff[1]);
}

void afficherMascotte() {

	char *turkey = "\n ----------------------------------------\n\
/ Bienvenue sur la contrefaçon Twitter ! \\\n\
| Une application au top de la           |\n\
\\ technologie !                          /\n\
 ----------------------------------------\n\
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

void afficher_liste(char *liste) {
    // Returns first token 
    char* token = strtok(liste, ","); 
  
    // Keep printing tokens while one of the 
    // delimiters present in liste. 
    while (token != NULL) { 
        printf("%s\n", token); 
        token = strtok(NULL, ","); 
    } 
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

void Recuperation(int sig) {
  quitter();
}


int suivre_utilisateur() {
	printf("\nNom de l'utilisateur à suivre : ");
	char name[MAXLINE];

	scanf("%s",name);

	envoyer(4,name);

	int recv_size;
    if ((recv_size = recv(serverSocket, bufRecv, MAXLINE, 0)) >= 2) {
		bufRecv[recv_size] = '\0';
		int type = bufRecv[0];
        char* message = &bufRecv[1];
         //affInfo(bufRecv);
        if (type==4 && message[0]=='\0') {
			printf("\nL'utilisateur est bien suivi\n");
        } else {
            printf("\nerreur : %s\n\n",message);
        }
    }
	viderBuffer();
	return 0;
}

int suivre_thematique() {
	printf("\nNom de la thématique à suivre : ");
	char name[MAXLINE];

	scanf("%s",name);

	envoyer(5,name);

	int recv_size;
    if ((recv_size = recv(serverSocket, bufRecv, MAXLINE, 0)) >= 2) {
		bufRecv[recv_size] = '\0';
		int type = bufRecv[0];
        char* message = &bufRecv[1];
         //affInfo(bufRecv);
        if (type==5 && message[0]=='\0') {
			printf("\nLa thématique est bien suivi\n");
        } else {
            printf("\nerreur : %s\n\n",message);
        }
    }
	viderBuffer();
	return 0;
}

int demander_utilisateurs_suivis() {
	int numpage = 1;
	char toSend[2];
	int arreter = 0;
	
	printf("\nVoici les utilisateurs que vous suivez :\n");
	
	while (!arreter) {
		toSend[0] = (char) numpage;
		toSend[1] = '\0';
		envoyer(6,toSend);

		int recv_size;
		if ((recv_size = recv(serverSocket, bufRecv, MAXLINE, 0)) >= 2) {
			bufRecv[recv_size] = '\0';
			int type = bufRecv[0];
			char* message = &bufRecv[1];
			//affInfo(bufRecv);
			if (type==6 && message[0]!='\0') {
				//affInfo(bufRecv);
				numpage++;
				afficher_liste(message);
			} else {
				arreter = 1;
			}
		}
	}
	printf("\n");
	return 0;
}

int demander_utilisateurs_qui_suivent() {
	int numpage = 1;
	char toSend[2];
	int arreter = 0;
	
	printf("\nVoici les utilisateurs qui vous suivent :\n");
	
	while (!arreter) {
		toSend[0] = (char) numpage;
		toSend[1] = '\0';
		envoyer(8,toSend);

		int recv_size;
		if ((recv_size = recv(serverSocket, bufRecv, MAXLINE, 0)) >= 2) {
			bufRecv[recv_size] = '\0';
			int type = bufRecv[0];
			char* message = &bufRecv[1];
			//affInfo(bufRecv);
			if (type==8 && message[0]!='\0') {
				//affInfo(bufRecv);
				numpage++;
				afficher_liste(message);
			} else {
				arreter = 1;
			}
		}
	}
	printf("\n");
	return 0;
}

int demander_thematiques_suivies() {
	int numpage = 1;
	char toSend[2];
	int arreter = 0;
	
	printf("\nVoici les thématiques que vous suivez :\n");
	
	while (!arreter) {
		toSend[0] = (char) numpage;
		toSend[1] = '\0';
		envoyer(7,toSend);

		int recv_size;
		if ((recv_size = recv(serverSocket, bufRecv, MAXLINE, 0)) >= 2) {
			bufRecv[recv_size] = '\0';
			int type = bufRecv[0];
			char* message = &bufRecv[1];
			//affInfo(bufRecv);
			if (type==7 && message[0]!='\0') {
				//affInfo(bufRecv);
				numpage++;
				afficher_liste(message);
			} else {
				arreter = 1;
			}
		}
	}
	printf("\n");
	return 0;
}

int attendre_tweet() {
	
	int recv_size;
	if ((recv_size = recv(serverSocket, bufRecv, MAXLINE, 0)) >= 2) {
		bufRecv[recv_size] = '\0';
		int type = bufRecv[0];
		char* message = &bufRecv[1];
		//affInfo(bufRecv);
		if (type==9 && message[0]!='\0') {
			//affInfo(bufRecv);
			printf("Tweet reçu :\n%s\n\n",message);
		}
	}
	
	return 0;
}

int twitter() {
	printf("\nEcrivez votre tweet :\n");
	char tweet[MAXLINE];

	fgets(tweet, MAXLINE, stdin);

	envoyer(3,tweet);

	int recv_size;
    if ((recv_size = recv(serverSocket, bufRecv, MAXLINE, 0)) >= 2) {
		bufRecv[recv_size] = '\0';
		int type = bufRecv[0];
        char* message = &bufRecv[1];
         //affInfo(bufRecv);
        if (type==3 && message[0]=='\0') {
			printf("\nLe tweet est bien envoyé\n");
        } else {
            printf("\nerreur : %s\n\n",message);
        }
    }
	return 0;
}


int menuConnecte() {
  char answer;
  int cont = 0;
  while (1) {
	  answer = '\0';
	  cont=0;
	  printf("\nQue voulez vous faire ?\n");
	  while (!cont) {
		printf("t -> twitter\na -> attendre un tweet\ns -> suivre un utilisateur\nr -> suivre une thématique\nu -> demander la liste des utilisateurs suivis\nv -> demander la liste des utilisateurs qui vous suivent\nh -> demander la liste des thématiques suivies\nq -> quitter l'application\nVotre choix : ");
		answer = getchar();
		if (answer && (answer=='t' || answer=='a' || answer=='s' ||answer=='r' ||answer=='u' || answer=='v' || answer=='h' || answer=='q')) {
		  cont = 1;
		} else {
		  printf("\nVotre entrée n'est pas correcte. Réessayez.\n");
		}
		viderBuffer();
	  }
	  switch (answer) {
		case 't' :
			twitter();
			break;
		case 'a' :
			attendre_tweet();
			break;
		case 's' :
			suivre_utilisateur();
		  break;
		case 'r' :
			suivre_thematique();
		  break;
		case 'u' :
		  demander_utilisateurs_suivis();
		  break;
		case 'v' :
		  demander_utilisateurs_qui_suivent();
		  break;
		case 'h' :
		  demander_thematiques_suivies();
		  break;
		case 'q' :
		  quitter();
		  break;
	  }
  }
  return 0;
}


int demande_connexion() {
  int Cbon=0;
  char name[MAXLINE];
  char password[MAXLINE];
  char *toSend;



  while(!Cbon) {
      printf("\nNom d'utilisateur : ");
      scanf("%s", name);
      printf("Mot de passe : ");
      system("stty -echo");
      scanf("%s", password);
      system("stty echo");

      if (name!=NULL && strlen(name)!=0) { //les données sont conformes

          toSend = malloc(sizeof(char)*(strlen(name)+strlen(password)+2));

          strcpy(toSend,name);
          strcat(toSend,"@");
          strcat(toSend,password);

          envoyer(1,toSend);

          int recv_size;
          if ((recv_size = recv(serverSocket, bufRecv, MAXLINE, 0)) >= 2) {
            bufRecv[recv_size] = '\0';
            int type = bufRecv[0];
            char* message = &bufRecv[1];
            //affInfo(bufRecv);
            if (type==1 && message[0]=='\0') {
              printf("\nLa connexion a réussi\n");
              Cbon = 1;
            } else {
              printf("\nerreur : %s\n\n",message);
            }
          }
      } else {
          printf("\nVos données sont non conformes\n");
      }
    }
    viderBuffer();
    menuConnecte();
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
		  printf("Votre nom d'utilisateur : ");
      scanf("%140s",nomUtil);
      printf("Votre mot de passe : ");
    	scanf("%140s",mdp);
    	printf("Confirmation du mot de passe : ");
    	scanf("%140s",confmdp);

    	if (strcmp(mdp,confmdp)==0 && nomUtil!=NULL && strlen(nomUtil)!=0) { //les données sont conformes

      		toSend = malloc(sizeof(char)*(strlen(nomUtil)+strlen(mdp)+2));

      		strcpy(toSend,nomUtil);
      		strcat(toSend,"@");
      		strcat(toSend,mdp);

      		envoyer(0,toSend);

      		if ((recv_size = recv(serverSocket, bufRecv, MAXLINE, 0)) >= 2) {
      			bufRecv[recv_size] = '\0';
				    int type = bufRecv[0];
				    char* message = &bufRecv[1];
				    //affInfo(bufRecv);
            if (type==0 && message[0]=='\0') {
              printf("\nLe compte est bien créé.\n");
              Cbon = 1;
            } else {
              printf("\nerreur : %s\n\n",message);
            }
      		}


      } else {
      		printf("\nVos données sont non conformes\n");
      }
  	}
  	viderBuffer();
    menuConnecte();
}


int connexion(char *addIp, int port) {

	//on remplis la structure serv_addre avec l'adresse du serveur
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin6_family = AF_INET6;
	int portno = port;
    serv_addr.sin6_port = htons(portno);
	inet_pton(AF_INET6, "::1", &serv_addr.sin6_addr);

	//on crée la socket
	if((serverSocket = socket(AF_INET6,SOCK_STREAM,0))<0){
		perror("erreur socket");
		exit(1);
	}
	printf("avant connect");
	//on connecte la socket
	if(connect(serverSocket,(struct sockaddr *) &serv_addr,sizeof(serv_addr))<0){
		perror("erreur connect");
		exit(1);
	}
	printf("apres connect");
  return 0;
}


int menuPrincipal() {
	  /* lance le menu principal de l'appli
  */
	char answer;
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
          demande_connexion();
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
	char *addIp = "::1";
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

  struct sigaction nvt, old;
  memset(&nvt,0,sizeof(nvt));
  /* memset necessaire pour flags=0 */
  nvt.sa_handler = Recuperation;
  sigaction(SIGINT,  &nvt, &old);

	connexion(addIp, port);

	afficherMascotte();
	menuPrincipal();

	quitter();
	return 0;
}

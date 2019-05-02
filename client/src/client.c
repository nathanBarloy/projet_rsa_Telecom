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

#define _XOPEN_SOURCE 600
#define MAXLINE 80
#define TAILLEMAX 20

/*
Serveur TCP :

Socket => Bind => Listen => Accept => Exchange (reader/writer) => Close
*/



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




void usage(){
	printf("usage : clieecho adresse_ip_server numero_port_serveur \n");
}

int writen (fd, ptr, nbytes)
     int  fd;
     char *ptr;
     int nbytes;
{
  int nleft, nwritten; 
  char *tmpptr;

  nleft = nbytes;
  tmpptr=ptr;
  while (nleft >0) {
    nwritten = write(fd,ptr, nleft);
    if (nwritten <=0) {
      if(errno == EINTR)
  nwritten=0;
      else{
  perror("probleme  dans write\n");
  return(-1);
      }
    }
    nleft -= nwritten;
    ptr += nwritten;
  }
  return (nbytes);
}


/*
 * Lire  "n" octets à partir d'un descripteur de socket
 */
int readn (fd, ptr, maxlen)
     int  fd;
     char *ptr;
     int maxlen;
{
  char *tmpptr;
  int nleft, nreadn;

  nleft = maxlen;
  tmpptr=ptr;
  
  while (nleft >0) {
    nreadn = read (fd,ptr, nleft);
    if (nreadn < 0) {
      if(errno == EINTR)
  nreadn=0;
      else{
  perror("readn : probleme  dans read \n");
  return(-1);
      }
    }
    else if(nreadn == 0){
      /* EOF */ 
      break ;
    }
    nleft -= nreadn;
    ptr += nreadn;
  }
  return (maxlen - nleft);
}

/*
 * Lire  une ligne terminee par \n à partir d'un descripteur de socket
 */
int readline (fd, ptr, maxlen)
     int  fd;
     char *ptr;
     int maxlen;
{
  
  int n, rc, retvalue, encore=1;  char c, *tmpptr; 

  tmpptr=ptr;
  for (n=1; (n < maxlen) && (encore) ; n++) {
    if ( (rc = read (fd, &c, 1)) ==1) {
      *tmpptr++ =c; 
      if (c == '\n')  /* fin de ligne atteinte */
  {encore =0; retvalue = n;}
    }
    else if (rc ==0) {  /* plus rien à lire */
      encore = 0;
      if (n==1) retvalue = 0;  /* rien a été lu */
      else retvalue = n;
    }
    else { /*rc <0 */
      if (errno != EINTR) {
  encore = 0;
  retvalue = -1;
      }
    }
  }
  *tmpptr = '\0';  /* pour terminer la ligne */
  return (retvalue);
}

int menuConnecte(char *addIp, int port) {
  char answer;
  int cont = 0;
  printf("\nBienvenue sur le Twitter du pauvre ! Que voulez vous faire ?\n");
  while (!cont) {
    printf("t -> twitter\nu -> demander la liste des utilisateurs suivis\nv -> demander la liste des utilisateurs qui vous suivent\nh -> demander la liste des thématiques suivies\nd -> déconnexion\nq -> quitter l'application\nVotre choix : ");
    answer = getchar();
    if (answer && (answer=='t' || answer=='u' || answer=='v' || answer=='h' || answer=='d' || answer=='q')) {
      cont = 1;
    } else {
      printf("\nVotre entrée n'est pas correcte. Réessayez.\n");
    }
    while (getchar()!='\n'); //pour vider la stdin
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
      printf("quitter\n");
      exit(0);
      break;
  }

  return 0;
}



int connexion(char *addIp, int port) {
  /*se connecte au serveur situé à l'adresse désignée par les paramètres*/

  int serverSocket,servlen,n,retread;
  struct sockaddr_in serv_addr;
  char fromServer[MAXLINE];
  char fromUser[MAXLINE];
  struct hostent *hp;

//on remplis la structure serv_addr avec l'adresse du serveur
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  int portno = port;
      serv_addr.sin_port = htons(portno);
  hp=(struct hostent*)gethostbyname(addIp);
  if(hp==NULL){
    fprintf(stderr,"client : %s ,p, trouve dans in /etc/hosts ou dans le DNS\n",addIp);
    exit(0);
  }
  serv_addr.sin_addr = *((struct in_addr *)(hp->h_addr));
  printf("IP address :%s\n",inet_ntoa(serv_addr.sin_addr));
  
  /*
  ouvrire socket (socket stream)
  */

  if((serverSocket = socket(AF_INET,SOCK_STREAM,0))<0){
    perror("erreur socket");
    exit(1);
  }
  printf("socket ok\n");
  
  if(connect(serverSocket,(struct sockaddr *) &serv_addr,sizeof(serv_addr))<0){
    perror("erreur connect");
    exit(1);
  }
  printf("connect ok\n");

  while((retread=readline(serverSocket,fromServer,MAXLINE))>0){
    printf("corr : %s", fromServer);
    if(strcmp(fromServer,"Au revoir\n")==0){
      break;
    }
    printf("vous : ");
    if(fgets(fromUser,MAXLINE,stdin)==NULL){
      perror("erreur fgets\n");
      exit(1);
    }
    //Envoyer le message au server
      if((n=writen(serverSocket,fromUser,strlen(fromUser))) != strlen(fromUser)){   
      printf("erreur writen");
      exit(0);
    }
  }
  if(retread <0){
    perror("erreur readline\n");
  }
  close(serverSocket);
  return 0;
}


int menuPrincipal(char *addIp, int port) {
  /* lance le menu principal de l'appli
  */
  char answer;
  char name[TAILLEMAX];
  char password[TAILLEMAX];
  int cont = 0;

  printf("\nBienvenue sur le Twitter du pauvre ! Que voulez vous faire ?\n");
  while (!cont) {
    printf("c -> connexion\nn -> créer un nouveau compte\nq -> quitter l'application\nVotre choix : ");
    answer = getchar();
    if (answer && (answer=='c' || answer=='n' || answer=='q')) {
      cont = 1;
    } else {
      printf("\nVotre entrée n'est pas correcte. Réessayez.\n");
    }
    while (getchar()!='\n'); //pour vider la stdin
  }
  switch (answer) {
    case 'c' :
      
      printf("Nom d'utilisateur : ");
      fflush(stdout);
      scanf("%s", name);
      fflush(stdout);
      printf("Mot de passe : ");
      fflush(stdout);
      scanf("%s", password);
      connexion(addIp,port);
      break;
    case 'n' :
      printf("nouveau\n");
      break;
    case 'q' :
      printf("quitter\n");
      exit(0);
      break;
  }

  return 0;
  
}



int main(int argc, char **argv){

  char *addIp = "127.0.0.1";
  int port = 8080;

  switch (argc) {
    case 1 :
      break;
    case 2 :
      port = atoi(argv[1]);
      break;
    case 3 :
      port = atoi(argv[1]);
      addIp = argv[2];
      break;
    default :
      printf("Trop d'arguments\n");
      exit(1);
  }

  menuPrincipal(addIp, port);

  return 0;
}


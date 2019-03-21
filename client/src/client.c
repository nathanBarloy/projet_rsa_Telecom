#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>

#define MAXLINE 80

/*
Serveur TCP :

Socket => Bind => Listen => Accept => Exchange (reader/writer) => Close
*/

void usage(){
	printf("usage : clieecho adresse_ip_server numero_port_serveur \n");
}



int connexion(char *addIp, int port) {
  int serverSocket,servlen,n,retread;
  struct sockaddr_in serv_addr;
  char fromServer[MAXLINE];
  char fromUser[MAXLINE];
  struct hostent *hp;

//on remplis la structure serv_addre avec l'adresse du serveur
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
  
  if(connect(serverSocket,(struct sockaddr *) &serv_addr,sizeof(serv_addr))<0){
    perror("erreur connect");
    exit(1);
  }

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
}





int main(int argc, char **argv){

  char *addIp = "127.0.0.1";
  int port = 8080;

	connexion(addIp, port);
  return 0;
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
    nwritten = write (fd,ptr, nleft);
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



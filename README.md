# MyTwitter

Objectifs :

1. créer un protocole de connexion et de déconnexion TCP en muliclient

2. permettre l'authentification et la création de compte (1ère BDD pour la liste des utilisateurs)

3. envoie et réception de données (un simple echo pour l'instant)

4. (coté serveur) mettre en place la BDD des utilisateurs et thématiques suivis.

4. (coté client) stocker les tweets recus.


Les noms et les thématiques ne contiennent que des lettres, chiffres, ainsi que les caractères '_' et '-'

protocole d'échange utilisé : un entier indiquant le type de message, suivi du contenu

client :

* 0 : demande de création d'un compte, de la forme nom\@motdepasse
* 1 : authentification, idem que 0
* 2 : désauthentification,vide
* 3 : envoi d'un tweet, chaine de caractères (140 max)
* 4 : demande de suivi d'un utilisateur, nom
* 5 : demande de suivi d'une thématique, nom
* 6 : demande de la liste des utilisateurs suivis, vide
* 7 : demande de la liste des utilisateurs qui vous suivent, vide
* 8 : demande de la liste des thématiques suivies, vide


serveur :

* 0 : accepter la création du compte, erreur
* 1 : accepter l'authentification du compte, erreur
* 2 : ack désauthentification,vide
* 3 : ack d'envoi de tweet
* 4 : accepter suivi de l'utilisateur, erreur
* 5 : accepter suivi d'une thématique, erreur
* 6 : envoi liste utilisateurs suivis, liste des noms séparés par ','
* 7 : envoi liste utilisateurs qui vous suivent, liste des noms séparés par ','
* 8 : envoi liste thématiques suivies, liste des noms séparés par ','
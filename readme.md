# *MyTwitter*

# Manuel d'utilisation

## Construction

Après avoir cloné le dépôt, construisez le client et le serveur en exécutant la commande suivante à la racine du projet :

```shell
$ make build
```

## Client

Pour lancer un client (écoutant le port `2222` du serveur à l'adresse `127.0.0.1` par défaut), exécutez la commande à la racine du projet :
```shell
$ ./client
```

Il est possible de spécifier une autre adresse comme ceci :

```shell
$ ./client 192.168.1.1
```

Il est également possible de spécifier un autre port comme ceci :

```shell
$ ./client 192.168.1.1 4444
```

## Serveur

Pour lancer un serveur (communiquant par le port `2222` par défaut), exécutez la commande à la racine du projet :
```shell
$ ./server
```

Il est possible de spécifier un autre port comme ceci :

```shell
$ ./server 4444
```

# Spécification du protocole applicatif

## Généralités

Les messages envoyés par le client au serveur ou l'inverse ont un format similaire. Leur taille est comprise entre `2` et `142` octets :

- le premier octet représente le type de message et est interprété comme un entier (non signé) ;
- le dernier octet doit être nul ;
- les octets médians sont non nuls et constituent l'information principale du message, dont l'interprétation diffère selon le type de message.

Certains messages font intervenir des identifiants, noté `<id>`. Par identifiant, on entend une chaîne de caractères  alphanumériques *ASCII*, de caractères `_` ou de caractères `-`.

À une requête de type `i` de la part du client doit suivre une réponse de même type `i` de la part du serveur, en guise d'acquittement, mais le serveur peut envoyer certains messages au client sans avoir reçu de demande de sa part (par exemple le type `9`).

### Format des messages du client au serveur

On dispose dans cette version de `9` types de messages différents :

- `0` (demande de création de compte) : les octets médians sont interprétés comme une chaîne de caractères et doivent respecter le format `<username>@<password>`, où `<username>` est un `<id>` (non vide) représentant le nom d'utilisateur et `<password>` un `<id>` (possiblement vide) représentant le mot de passe ;
- `1` (demande d'authentification) : même format que pour le type `0` ;
- `2` (demande de déconnexion) : les octets médians sont ignorés ;
- `3` (envoi de *tweet*) : les octets médians sont interprétés comme une chaîne de caractères représentant le contenu du *tweet*, dont chaque sous-chaîne de la forme `#<tag>`, où `<tag>` est un `<id>`, (non vide et de taille maximale) représente une citation de la thématique de nom `<tag>` ;
- `4` (demande de suivi d'utilisateur) : les octets médians sont interprétés comme une chaîne de caractères et doivent respecter le format `<username>`, où `<username>` est un `<id>` (non vide) représentant un nom d'utilisateur ;
- `5` (demande de suivi de thématique) : les octets médians sont interprétés comme une chaîne de caractères et doivent respecter le format `<tag>` où `<tag>` est un `<id>` (non vide) représentant un nom de thématique ;
- `6` (demande de listage des utilisateurs suivis) : le nombre d'octets médians doit être `1` et l'unique octet médian est interprété comme un entier (non signé et non nul) représentant le numéro de page ;
- `7` (demande de listage des thématiques suivies) : même format que pour le type `6` ;
- `8` (demande de listage des utilisateurs suivants) : même format que pour le type `6`.

Le format des autres types de messages n'est pas défini.

### Format des messages du serveur au client

On dispose dans cette version de `10` types de messages différents :

- `0` (acquittement de demande de création de compte) : le nombre d'octets médians doit être `0` si et seulement si la création de compte est désormais effective ;
- `1` (acquittement de demande d'authentification) : le nombre d'octets médians doit être `0` si et seulement si l'authentification est désormais effective ;
- `2` (notification de déconnexion) : le nombre d'octets médians doit être `0` si et seulement si la déconnexion est désormais effective ;
- `3` (acquittement d'envoi de *tweet*) : le nombre d'octets médians doit être `0` si et seulement si l'envoi de *tweet* est permis ;
- `4` (acquittement de demande de suivi d'utilisateur) : le nombre d'octets médians doit être `0` si et seulement si le suivi est désormais effectif ;
- `5` (acquittement de demande de suivi de thématique) : même format que pour le type `4` ;
- `6` (réponse à demande de listage des utilisateurs suivis) : les octets médians sont interprétés comme une chaîne de caractères et doivent respecter le format `(<username>(,<username>)*)?` où `<username>` est un `<id>` (non vide) représentant un nom d'utilisateur ;
- `7` (réponse à demande de listage des thématiques suivies) : les octets médians sont interprétés comme une chaîne de caractères et doivent respecter le format `(<tag>(,<tag>)*)?` où `<tag>` est un `<id>` (non vide) représentant un nom de thématique ;
- `8` (réponse à demande de listage des utilisateurs suivants) : même format que pour le type `6` ;
- `9` (notification de réception de *tweet*) : même format que pour le type `3` décrit dans la partie précédente.

Le format des autres types de messages n'est pas défini.

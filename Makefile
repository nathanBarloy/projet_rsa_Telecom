CC = gcc # Compilateur
CFLAGS = -Wall -g # Options de compilation
EXEC = ./client/src/client ./server/src/server
SERVER = ./server/src/server
CLIENT = ./client/src/client

all : $(EXEC)

client : $(CLIENT)

server : $(SERVER)

%: %.c                       # Règle de compilation pour chaque fichier C
	$(CC) $(CFLAGS) $< -o $@

cleanClient :
	rm -rf client/src/*.o

cleanServer :
	rm -rf server/src/*.o

clean :
	rm -rf server/src/*.o
	rm -rf client/src/*.o

mrproper :
	rm -rf $(EXEC)

.PHONY : client

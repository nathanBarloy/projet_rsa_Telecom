build: client.c server.c
	gcc -Wall client.c -o client
	gcc -Wall server.c -o server

clean:
	rm -f client server

.PHONY: all clean

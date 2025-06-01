CC = gcc
CFLAGS = -Wall
SERVER_OBJS = server/main.c
CLIENT_OBJS = client/client.c

all: server client

server: $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o server/server $(SERVER_OBJS)

client: $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o client/client $(CLIENT_OBJS)

clean:
	rm -f server/server client/client


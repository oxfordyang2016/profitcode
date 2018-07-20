.PHONY:clean

all:example-client example-server

example-client:example-client.c
	gcc example-client.c -o example-client -lpthread -lwebsockets

example-server:example-server.c
	gcc example-server.c -o example-server -lwebsockets

clean:
	-rm example-client example-server

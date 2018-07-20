cpp = g++

client: ipc_client.cpp
	$(cpp) ipc_client.cpp -lzmq -o client
	$(cpp) ipc_server.cpp -lzmq -o server

clean:
	rm client server

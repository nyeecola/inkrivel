all: client server

folder:
	mkdir -p bin

client: folder
	g++ src/chat_client.cpp -o bin/chat_client -lpthread

server: folder
	g++ src/chat_server.cpp -o bin/chat_server -lpthread

run_client: client
	bin/chat_client

run_server: server
	bin/chat_server



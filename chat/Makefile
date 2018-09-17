all: client server

client:
	g++ src/chat_client.cpp -o bin/chat_client -lpthread

server:
	g++ src/chat_server.cpp -o bin/chat_server -lpthread

run_client: client
	bin/chat_client

run_server: server
	bin/chat_server



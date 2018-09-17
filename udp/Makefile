compile_client:
	g++ src/udp_client.cpp -o bin/udp_client -Wall -Werror

compile_server:
	g++ src/udp_server.cpp -o bin/udp_server -Wall -Werror

run_client: compile_client
	./bin/udp_client

run_server: compile_server
	./bin/udp_server

compile: compile_client compile_server
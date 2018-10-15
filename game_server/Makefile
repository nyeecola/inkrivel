compile: folder
	g++ src/game_server.cpp -o bin/game_server -O3 -Wall -lSDL2 -lSDL2_image -lGL -lpthread -g

folder:
	mkdir -p bin

run: compile
	./bin/game_server

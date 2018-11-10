compile: folder
	g++ src/game_server.cpp -o bin/game_server -O3 -Wall -lSDL2 -lSDL2_image -lGL -lpthread -g

folder:
	mkdir -p bin

run: compile
	./bin/game_server 8 0 0 1 1 2 2 3 3

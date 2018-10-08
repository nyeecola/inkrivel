compile: folder
	g++ src/game_server.cpp -o bin/game_server -Wall -lSDL2 -lSDL2_image -lGL

folder:
	mkdir -p bin

run: compile
	./bin/game_server

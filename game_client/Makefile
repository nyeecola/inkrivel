compile: folder
	g++ src/game_client.cpp -o bin/game_client -Wall -lSDL2 -lSDL2_image -lGL -g

folder:
	mkdir -p bin

run: compile
	./bin/game_client 0

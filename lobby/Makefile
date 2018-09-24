all:
	mkdir -p bin
	g++ main.cpp gl3w/gl3w.c -o bin/main -lglfw -lm -ldl -lpthread

run: all
	./bin/main

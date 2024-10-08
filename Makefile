all:
	g++ src/*.cpp src/*.c -o out -Iinclude -Llib -lSDL2 -lSDL2_image -lglm
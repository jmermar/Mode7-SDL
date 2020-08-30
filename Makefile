mode7:	main.cpp
	g++ -o mode7 main.cpp `sdl2-config --libs --cflags`

clean:
	rm -f mode7

dep:
	sudo apt install libsdl2-dev
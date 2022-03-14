LIBS	:= -lSDL2main -lSDL2 -lm
SOURCE	:= lsystem.cpp
FLAGS	:= -O3 -Wall

run:
	g++ $(FLAGS) $(SOURCE) $(LIBS)
	./a.out
	rm a.out

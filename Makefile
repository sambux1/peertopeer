# using -Og compiler optimization to make debugging easier
COMPILER_FLAGS = -Og

default :
	g++ $(COMPILER_FLAGS) peer.cpp main.cpp

clean :
	rm *.out
# using -Og compiler optimization to make debugging easier
COMPILER_FLAGS = -Og

default :
	g++ $(COMPILER_FLAGS) src/peer.cpp main.cpp

clean :
	rm *.out
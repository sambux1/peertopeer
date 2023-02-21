# using -Og compiler optimization to make debugging easier
COMPILER_FLAGS = -Og

default :
	g++ $(COMPILER_FLAGS) src/peer.cpp main.cpp

test :
	g++ tests/test.cpp src/peer.cpp -o test.out -I src/

clean :
	rm *.out
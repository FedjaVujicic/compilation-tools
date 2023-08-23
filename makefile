all: flex bison compile

flex:
	flex -o misc/lexer.cpp misc/lexer.l

bison:
	bison -d -o misc/parser.cpp misc/parser.y

compile:
	g++ misc/*.cpp src/*.cpp

clean:
	rm misc/lexer.cpp misc/parser.cpp misc/parser.hpp a.out
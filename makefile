all:
	flex -o lexer.cpp lexer.l && bison -d -o parser.cpp parser.y && g++ lexer.cpp parser.cpp assembler.cpp

clean:
	rm lexer.cpp parser.cpp parser.hpp a.out
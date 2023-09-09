all: flex bison compile_as

flex:
	flex -o misc/lexer.cpp misc/lexer.l

bison:
	bison -d -o misc/parser.cpp misc/parser.y

compile_as:
	g++ -o asembler misc/lexer.cpp misc/parser.cpp src/assembler.cpp src/assembler_main.cpp src/section.cpp src/symbol.cpp

clean:
	rm misc/lexer.cpp misc/parser.cpp misc/parser.hpp asembler *.o
all: flex bison compile_as compile_lk compile_em

flex:
	flex -o misc/lexer.cpp misc/lexer.l

bison:
	bison -d -o misc/parser.cpp misc/parser.y

compile_as:
	g++ -o assembler misc/lexer.cpp misc/parser.cpp src/assembler.cpp src/assembler_main.cpp src/symbol.cpp

compile_lk:
	g++ -o linker src/linker.cpp src/linker_main.cpp src/symbol.cpp

compile_em:
	g++ -o emulator src/emulator_main.cpp src/emulator.cpp

clean:
	rm misc/lexer.cpp misc/parser.cpp misc/parser.hpp assembler linker emulator *.o *.hex
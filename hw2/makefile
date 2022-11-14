Main: main.cpp my_print.asm
	nasm -f elf32 my_print.asm -g -o build/my_print.o
	g++ -m32 main.cpp build/my_print.o -o build/main -std=c++11 -g
	rm -rf build/my_print.o
clean:
	rm -rf build/*

run: Main

	build/main

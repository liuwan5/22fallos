
build:
	nasm -f elf32 -g -o hw1.bin hw1.asm && ld -m elf_i386 hw1.bin -o hw1.out

test: hw1.out
	python3 test.py

clean:
	@rm -rf hw1.bin hw1.out

build:
	nasm -f elf32 -g -o cal.bin cal.asm && ld -m elf_i386 cal.bin -o cal.out

test: cal.out
	python3 test.py

clean:
	@rm -rf cal.bin cal.out
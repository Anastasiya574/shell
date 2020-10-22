all:
	mkdir bin -p
	gcc source/main.c -o bin/main -Wall -Werror -lm -fsanitize=leak    

clean:
	rm bin/main
	rmdir bin

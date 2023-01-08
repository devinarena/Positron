all:
	gcc src/*.c -o program -Wall -Wextra -g

debug:
	gcc src/*.c -o program -Wall -Wextra -g
	./program input.pt
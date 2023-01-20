all:
	gcc src/*.c -o positron -Wall -Wextra -g

debug:
	gcc src/*.c -o positron -Wall -Wextra -g
	./positron -d input.pt

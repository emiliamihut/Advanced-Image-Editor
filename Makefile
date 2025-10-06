COMP = gcc
FLAGS = -Wall -Wextra

PROBLEM = image_editor
build: $(PROBLEM)

image_editor: image_editor.c
	$(COMP) $(FLAGS) image_editor.c -o image_editor -lm

clean:
	rm -f $(PROBLEM) *.o
	
valgrind:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./image_editor
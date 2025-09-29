maefuck: main.c
	gcc main.c -o maefuck -ggdb

run: maefuck
	./maefuck

clean:
	rm maefuck

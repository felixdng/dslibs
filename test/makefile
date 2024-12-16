INC_DIR:=-I../inc
CFLAGS:=-Wall -O0 -g --std=gnu99
LDFLAGS:=-L../build -ldstool

all:
	gcc $(CFLAGS) $(INC_DIR) cmd_main.c cmd_func.c -o cmd-demo $(LDFLAGS)

main:
	gcc $(CFLAGS) $(INC_DIR) main.c -o demo $(LDFLAGS)

clean:
	rm -f cmd-demo demo

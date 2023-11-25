CFLAGS= -g 
LDFLAGS= -lncurses 
CC=gcc

PWD=`pwd`

TARGET=terris


${TARGET}: main.o
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)
main.o: main.c
	$(CC) -c $< $(CFLAGS) $(LDFLAGS)	


.PHONY: clean run 
run:
	./${TARGET}
clean: 
	-rm ${TARGET} *.o 






CC = gcc
CFLAGS = -g -Wall -I.
OBJS = lab01.o main.o
DEPS = lab01.h
TARGET = lab01


lab01: $(OBJS)
	$(CC) $(OBJS) -o lab01

%.0 : %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)


	


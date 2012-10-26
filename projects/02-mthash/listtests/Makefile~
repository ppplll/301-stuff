CC = gcc
CFLAGS = -g -Wall -pthread
# main executable file first
TARGET = listtest
# object files next
OBJS = testlist.o  list.o
# header files next
DEPS =  list.h
.PHONY : clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -pthread -lm

.c.o: $(DEPS)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET) *~


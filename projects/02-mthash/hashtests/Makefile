CC = gcc
CFLAGS = -g -Wall -pthread
# main executable file first
TARGET = hashtest
# object files next
OBJS = hash.o list.o hashtest.o
# header files next
DEPS = hash.h list.h
.PHONY : clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) -pthread -lm

.c.o: $(DEPS)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET) *~


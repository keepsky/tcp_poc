CC = gcc
CFLAGS = -Wall -Wextra -O2
LIBS = -lws2_32
TARGET = server.exe
SRCS = server.c mongoose.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	.\$(TARGET)

clean:
	del /Q /F $(OBJS) $(TARGET)

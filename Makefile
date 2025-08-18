CC = gcc
CFLAGS = -Wall -O2 -Isrc
LDFLAGS = -lpthread -lssl -lcrypto

SRC_DIR = src
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:.c=.o)
TARGET = ytstats

PREFIX = /usr/share/nanohatoled

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

install: $(TARGET)
	sudo systemctl stop ytstats
	sudo cp $(TARGET) $(PREFIX)/$(TARGET)
	sudo chmod +x $(PREFIX)/$(TARGET)
	sudo systemctl start ytstats

clean:
	rm -f $(SRC_DIR)/*.o $(TARGET)

uninstall:
	sudo systemctl stop ytstats
	sudo rm -f $(PREFIX)/$(TARGET)
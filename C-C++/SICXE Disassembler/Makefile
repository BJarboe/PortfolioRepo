# Bryce Jarboe
# RedID 825033151
CC = g++
CFLAGS = -g -Wall
SRC = disassem.cpp
OBJ = disassem.o
TARGET = disassem
OUT = out.lst

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC)

clean:
	rm -f $(OBJ) $(TARGET) $(OUT)

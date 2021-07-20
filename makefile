SRC = $(wildcard ./*.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))

TARGET = server
CXX_FLAG = -g -Wall -std=c++11 -pthread -I .
CC = g++

$(TARGET) : $(OBJ)
	$(CC) $(CXX_FLAG) -o $@ $^

%.o : %.cpp
	$(CC) $(CXX_FLAG) -c $< -o $@

.PHONY : clean
clean :
	rm -rf *.o
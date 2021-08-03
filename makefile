BIN_DIR = ./
OBJ_DIR = ./obj
SRC_DIR = ./NetServer
INLUDE_DIR = ./NetServer

SRC = $(wildcard $(SRC_DIR)/*.cpp) # 搜索$(SRC_DIR)路径下的所有.cpp文件，生成以空格间隔的文件名列表
OBJ = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(notdir $(SRC))) # 在去除所有目录信息后的$(SRC)的文件列表中，将所有%.cpp文件替换成$(OBJ_DIR)/%.o文件

TARGET = net_server
CXX_FLAG = -g -Wall -std=c++11 -pthread -I $(INLUDE_DIR)
CC = g++

$(BIN_DIR)/$(TARGET) : $(OBJ)
	$(CC) $(CXX_FLAG) -o $@ $^

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CC) $(CXX_FLAG) -c $< -o $@

.PHONY : clean
clean :
	rm -rf $(OBJ_DIR)/*.o
init :
	if [ ! -d $(OBJ_DIR) ];	then mkdir -p $(OBJ_DIR); fi;
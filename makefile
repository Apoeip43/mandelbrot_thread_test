
CC=g++
STD=c++17

SRC_DIR=src
SRC_FILES=$(wildcard $(SRC_DIR)/*.cpp)

HEADER_DIR=headers
HEADER_FILES=$(wildcard $(HEADER_DIR)/*.h)

LIBS_DIR=libs
LIBS_FILES=$(patsubst $(HEADER_DIR)/%.h, $(LIBS_DIR)/%.o, $(HEADER_FILES))

CXXFLAGS= -I$(HEADER_DIR) -O3 -std=$(STD)

OUT=mandelthread

$(LIBS_DIR)/%.o: $(SRC_DIR)/%.cpp 
	$(CC) -c $^ -o $@ $(CXXFLAGS)

build: $(LIBS_FILES) $(LIBS_DIR)/main.o
	$(CC) $^ -o $(OUT) $(CXXFLAGS)

make bdbg: $(LIBS_FILES) $(LIBS_DIR)/main.o
	$(CC) $^ -o $(OUT)_dbg $(CXXFLAGS) -g3

.PHONY: clean build

clean:
	rm -rf $(wildcard $(LIBS_DIR)/*) $(OUT) $(OUT)_dbg


SRC_DIR := src
BIN_DIR := bin
INC_DIR := include

SFML := -lsfml-graphics -lsfml-window -lsfml-system

CPP_FILES := $(wildcard $(SRC_DIR)/*.cpp)
HPP_FILES := $(wildcard $(INC_DIR)/*.hpp)
EXE_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(BIN_DIR)/%.exe,$(CPP_FILES))

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/%.exe: $(SRC_DIR)/%.cpp $(HPP_FILES) | $(BIN_DIR)
	g++ -std=c++17 $< -o $@ $(SFML) -I$(INC_DIR)

all: $(EXE_FILES)

run: $(BIN_DIR)/main.exe
	./$(BIN_DIR)/main.exe

clean:
	rm -f $(EXE_FILES)

.PHONY: all clean run
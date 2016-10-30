CXX         ?= g++
CC          ?= gcc
CXXFLAGS     = $(INCLUDES_DIR) -std=c++14 -Wall -Wextra -Wpedantic -march=native -O3 -flto -pipe
CFLAGS       = $(CXXFLAGS)
INCLUDES_DIR = -I$(SRC_DIR)
BIN_DIR      = bin
OBJ_DIR      = obj
SRC_DIR      = src
TARGETS      = wav2stf
LIBS         = -lboost_system -lboost_filesystem -lfftw3

SOURCES := $(shell find $(SRC_DIR) -name *.cpp)
OBJECTS := $(SOURCES:$(SRC_DIR)%.cpp=$(OBJ_DIR)%.o)

.PHONY: all clean

all: $(addprefix $(BIN_DIR)/, $(TARGETS))

$(BIN_DIR)/wav2stf: $(addprefix $(OBJ_DIR)/, wav2stf/wav2stf.o util/utils.o util/wav_utils.o)
	mkdir -p $(BIN_DIR)
	$(CXX) $^ $(LIBS) $(CXXFLAGS) -o $(BIN_DIR)/wav2stf

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	$(CXX) $(CXXFLAGS) -MM $< > $@.d #generate dependency file
	sed -ir 's|.*:|$@:|' $@.d        #fix dependency's target

clean:
	rm -rf $(BIN_DIR)
	rm -rf $(OBJ_DIR)

-include $(OBJECTS:=.d) #pull in dependencies

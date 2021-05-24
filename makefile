# An ungly make file that works.
SRC_DIR = src
OBJ_DIR = obj

SRC_FILES := $(shell find $(SRC_DIR) -name "*.cpp")
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_FILES))

CFLAGS = -std=c++17 -fpermissive -Wno-narrowing -DSDL2 -DWIN95 "-D__cdecl=" -D__WATCOMC__ -Wall -I$(SRC_DIR)/ -I$(SRC_DIR)/m6502 -I$(SRC_DIR)/6502 -I$(SRC_DIR)/win95
LFLAGS = -lstdc++fs $$(pkg-config --cflags --libs sdl2) `sdl2-config --cflags --libs` -lSDL2_image

LINK = $(CXX) -o $@ $^ $(LFLAGS)

# The debug build.
nesticle: CFLAGS += -g
nesticle: $(OBJ_FILES)
	$(LINK)

# Delete objects.
clean:
	find obj/ -name "*.o" -type f -delete

# Object compiling.
MKDIR_P = mkdir -p

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(MKDIR_P) `dirname $@`
	$(CXX) $(CFLAGS) -c -o $@ $< $(LFLAGS)

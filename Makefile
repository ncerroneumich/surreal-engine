CXX = clang++
CXXFLAGS = -O3 -Wall -std=c++17 -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
INCLUDE_PATHS = -Iinclude
LIBRARIES = -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -llua5.4

SRC_DIR = src
BOX2D_SRC_DIR = src/collision src/common src/dynamics src/rope
BOX2D_SOURCES = $(foreach dir,$(BOX2D_SRC_DIR),$(wildcard $(dir)/*.cpp))
OBJ_DIR = obj

# Adjust OBJECTS to include Box2D object files correctly, replicating the directory structure under OBJ_DIR.
SOURCES = $(wildcard $(SRC_DIR)/*.cpp) $(BOX2D_SOURCES)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.cpp)) $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(BOX2D_SOURCES))

EXECUTABLE = game_engine_linux

all: dir $(EXECUTABLE)

dir:
	@mkdir -p $(OBJ_DIR)
	$(foreach dir,$(BOX2D_SRC_DIR),mkdir -p $(OBJ_DIR)/$(notdir $(dir));)

$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBRARIES)
	@echo "Build successful."

# General rule for object files, handling both main src and Box2D sources.
$(OBJ_DIR)/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATHS) -c $< -o $@

# Special rule for Box2D sources to maintain directory structure in OBJ_DIR.
$(OBJ_DIR)/collision/%.o: src/collision/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATHS) -c $< -o $@

$(OBJ_DIR)/common/%.o: src/common/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATHS) -c $< -o $@

$(OBJ_DIR)/dynamics/%.o: src/dynamics/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATHS) -c $< -o $@

$(OBJ_DIR)/rope/%.o: src/rope/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE_PATHS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(EXECUTABLE)
	@echo "Cleaned."

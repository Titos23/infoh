# ==========================================
#  Makefile - Build configuration (release flags)
# ==========================================

CXX = g++

# Compiler flags for release build (tuned for performance on common x86 CPUs)
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -march=native -mtune=native \
           -ffast-math -funroll-loops -finline-functions \
           -fomit-frame-pointer -ftree-vectorize -pthread

# LTO flag separate (causes issues in some systems)
LTO_FLAG = -flto

# Source files
SRC = main.cpp blastDatabase.cpp queryReader.cpp blosumMatrix.cpp smithWaterman.cpp
OBJ = $(SRC:.cpp=.o)
HEADERS = blastDatabase.h queryReader.h blosumMatrix.h smithWaterman.h

TARGET = projet

# Build
all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	@echo "=== Building with release flags ==="
	$(CXX) $(CXXFLAGS) $(LTO_FLAG) -o $(TARGET) $(SRC)
	@echo "✓ Build complete!"

clean:
	@echo "Cleaning..."
	rm -f $(OBJ) $(TARGET) *.gcda *.gcno

.PHONY: all clean
# ==========================================
#  Makefile for BLAST Protein Alignment Project
#  INFO-H-304 2025-2026
# ==========================================

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# --- Source files ---
SRC = main.cpp blastReader.cpp queryReader.cpp
OBJ = $(SRC:.cpp=.o)
HEADERS = blastReader.h queryReader.h

# ==========================================
# Targets for different versions
# ==========================================

# Preliminary version (just compile)
projetprelim: $(OBJ)
	@echo "=== Building projetprelim ==="
	$(CXX) $(CXXFLAGS) -o projetprelim $(OBJ)
	@echo "=== projetprelim ready! ==="

# Final version (just compile)
projet: $(OBJ)
	@echo "=== Building projet ==="
	$(CXX) $(CXXFLAGS) -o projet $(OBJ)
	@echo "=== projet ready! ==="

# Optimized version (just compile)
projetopt: $(OBJ)
	@echo "=== Building projetopt ==="
	$(CXX) $(CXXFLAGS) -O3 -march=native -o projetopt $(OBJ)
	@echo "=== projetopt ready! ==="

# ==========================================
# Compilation rules
# ==========================================
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ==========================================
# Cleanup targets
# ==========================================
clean:
	@echo "=== Cleaning object files and executables ==="
	rm -f *.o projetprelim projet projetopt

veryclean: clean
	@echo "=== Removing generated BLAST database files ==="
	rm -f database/*.pin database/*.psq database/*.phr

.PHONY: projetprelim projet projetopt clean veryclean
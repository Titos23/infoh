
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# --- Source files ---
SRC = main.cpp blastReader.cpp queryReader.cpp
OBJ = $(SRC:.cpp=.o)
HEADERS = blastReader.h queryReader.h

# ==========================================
# Required targets (compile only)
# ==========================================

# Preliminary version (intermediate deadline)
projetprelim: $(OBJ)
	
	$(CXX) $(CXXFLAGS) -o projetprelim $(OBJ)
	

# Final version 
projet: $(OBJ)
	@echo "=== Building projet ==="
	$(CXX) $(CXXFLAGS) -o projet $(OBJ)
	

# Optimized version 
projetopt: $(OBJ)
	@echo "=== Building projetopt ==="
	$(CXX) $(CXXFLAGS) -O3 -march=native -o projetopt $(OBJ)
	@echo "=== projetopt ready! ==="

# ==========================================
# Compilation rules
# ==========================================
%.o: %.cpp $(HEADERS)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ==========================================
# Cleanup targets
# ==========================================
clean:
	rm -f *.o projetprelim projet projetopt

veryclean: clean

# ==========================================
# Phony targets
# ==========================================
.PHONY: projetprelim projet projetopt clean veryclean
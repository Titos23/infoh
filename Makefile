# ==========================================
#  Makefile for BLAST Project (Projet Prelim)
#  Generates BLAST DB (version 4) from FASTA
#  Database files: uniprot_sprot.fasta.pin/.psq/.phr
# ==========================================

CXX      = g++
CXXFLAGS = -std=c++17 -Wall -O2

# --- Executable name ---
PRELIM = projetprelim

# --- Source files ---
SRC = main.cpp blastReader.cpp queryReader.cpp
OBJ = $(SRC:.cpp=.o)

# --- Database paths (keep .fasta in basename) ---
DB_FASTA = database/uniprot_sprot.fasta
DB_PIN   = $(DB_FASTA).pin
DB_PSQ   = $(DB_FASTA).psq
DB_PHR   = $(DB_FASTA).phr

# ==========================================
# Default target: build executable (and ensure DB exists)
# testprelim runs: make projetprelim
# ==========================================
$(PRELIM): $(OBJ) | db       # db is an order-only prerequisite
	@echo "=== Building $(PRELIM) ==="
	$(CXX) $(CXXFLAGS) -o $@ $^

# Optionally: make without args will also build projetprelim
all: $(PRELIM)


# ==========================================
# Compilation rules
# ==========================================
%.o: %.cpp
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ==========================================
# Cleanup
# ==========================================
clean:
	@echo "=== Cleaning object files and executables ==="
	rm -f *.o projetprelim projet projetopt


.PHONY:

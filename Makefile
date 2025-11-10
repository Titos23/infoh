# ==========================================
#  Makefile for BLAST Project (Projet Prelim)
#  Generates BLAST DB (version 4 only)
# ==========================================

CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2

# --- Executable name ---
PRELIM = projetprelim

# --- Source files ---
SRC = main.cpp blastReader.cpp queryReader.cpp
OBJ = $(SRC:.cpp=.o)

# --- Database and query paths ---
DB_FASTA = database/uniprot_sprot.fasta
DB_PIN   = $(DB_FASTA).pin
DB_PSQ   = $(DB_FASTA).psq
DB_PHR   = $(DB_FASTA).phr
QUERY    = query/P00533.fasta

# ==========================================
# Default target: build executable + DB v4 + run
# ==========================================
all: $(PRELIM) $(DB_PIN) $(DB_PSQ) $(DB_PHR)
	@echo "=== Running projetprelim ==="
	./$(PRELIM) $(QUERY) $(DB_FASTA)

# ==========================================
# Build executable
# ==========================================
$(PRELIM): $(OBJ)
	@echo "=== Building $(PRELIM) ==="
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ==========================================
# Generate BLAST database (version 4)
# ==========================================
$(DB_PIN) $(DB_PSQ) $(DB_PHR): $(DB_FASTA)
	@echo "=== Generating BLAST database (version 4) ==="
	@if [ -x ./makeblastdb ]; then \
		echo "Using local makeblastdb..."; \
		./makeblastdb -in $(DB_FASTA) -dbtype prot -blastdb_version 4; \
	elif command -v makeblastdb >/dev/null 2>&1; then \
		echo "Using system makeblastdb..."; \
		makeblastdb -in $(DB_FASTA) -dbtype prot -blastdb_version 4; \
	else \
		echo "Error: makeblastdb not found!"; \
		echo "Please install BLAST+ or place makeblastdb in this folder."; \
		exit 1; \
	fi
	@echo "=== Database successfully generated (version 4)! ==="

# ==========================================
# Cleanup targets
# ==========================================
clean:
	@echo "=== Cleaning object files and binary ==="
	rm -f *.o $(PRELIM)

veryclean: clean
	@echo "=== Removing generated BLAST database files ==="
	rm -f $(DB_FASTA).pin $(DB_FASTA).psq $(DB_FASTA).phr $(DB_FASTA).pjs $(DB_FASTA).pot

.PHONY: all clean veryclean

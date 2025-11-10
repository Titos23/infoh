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
DB_BASE  = $(basename $(DB_FASTA))     # database/uniprot_sprot
DB_PIN   = $(DB_BASE).pin
DB_PSQ   = $(DB_BASE).psq
DB_PHR   = $(DB_BASE).phr
QUERY    = query/P00533.fasta

# ==========================================
# Default target: build executable + DB v4 + run
# ==========================================
all: $(PRELIM) db
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
# BLAST database generation (version 4)
# ==========================================
db: $(DB_FASTA)
	@echo "=== Generating BLAST database (version 4) ==="
	@if [ -x ./makeblastdb ]; then \
		echo "Using local makeblastdb..."; \
		./makeblastdb -in $(DB_FASTA) -dbtype prot -blastdb_version 4 -out $(DB_BASE); \
	elif command -v makeblastdb >/dev/null 2>&1; then \
		echo "Using system makeblastdb..."; \
		makeblastdb -in $(DB_FASTA) -dbtype prot -blastdb_version 4 -out $(DB_BASE); \
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
	rm -f $(DB_BASE).pin $(DB_BASE).psq $(DB_BASE).phr $(DB_BASE).pjs $(DB_BASE).pot

.PHONY: all clean veryclean db

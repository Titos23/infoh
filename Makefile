# ==========================================
#  Makefile for BLAST Project (Projet Prelim)
#  Generates BLAST DB version 4 automatically
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
# Default target: compile, generate DB v4, then run
# ==========================================
all: $(PRELIM) $(DB_PIN) $(DB_PSQ) $(DB_PHR)
	@echo "=== Running projetprelim ==="
	./$(PRELIM) $(QUERY) $(DB_FASTA)

# ==========================================
# Compilation rules
# ==========================================
$(PRELIM): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ==========================================
# BLAST database generation (version 4 only)
# ==========================================
$(DB_PIN) $(DB_PSQ) $(DB_PHR): $(DB_FASTA)
	@echo "=== Generating BLAST database files (version 4) ==="
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
	rm -f *.o $(PRELIM)

veryclean: clean
	rm -f $(DB_PIN) $(DB_PSQ) $(DB_PHR)

.PHONY: all clean veryclean

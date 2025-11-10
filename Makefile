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
# BLAST database generation (version 4)
# Files: *.fasta.pin / *.fasta.psq / *.fasta.phr
# ==========================================
.PHONY: db
db: $(DB_PIN) $(DB_PSQ) $(DB_PHR)

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

veryclean: clean
	@echo "=== Removing generated BLAST database files (keeping .fasta) ==="
	rm -f \
		$(DB_FASTA).pin \
		$(DB_FASTA).psq \
		$(DB_FASTA).phr \
		$(DB_FASTA).pjs \
		$(DB_FASTA).pot \
		$(DB_FASTA).ptf \
		$(DB_FASTA).pto

.PHONY:

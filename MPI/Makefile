# ======================== Compiler ========================
# Use mpicc for MPI programs
CC = mpicc

# Compiler flags: optimize, show all warnings
CFLAGS = -O2 -Wall

# ======================== Source Files ========================
# Automatically collect all C source files in current dir
SRC = $(wildcard *.c)

# Corresponding object files (not strictly needed but included)
OBJ = $(SRC:.c=.o)

# Executable names (same as source filenames without .c)
EXE = $(SRC:.c=)

# ======================== Default Target ========================
all: $(EXE)

# ======================== Build Rule ========================
# Compile each .c file into executable
%: %.c
	$(CC) $(CFLAGS) -o $@ $<

# ======================== Run Helper ========================
# Just prints usage info for convenience
run:
	@echo "Usage: make <program_name> or ./<program_name>"

# ======================== Clean ========================
# Remove all compiled executables and object files
clean:
	rm -f $(OBJ) $(EXE)

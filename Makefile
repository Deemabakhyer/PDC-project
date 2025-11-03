CC = gcc-13
CFLAGS = -fopenmp -O2 -Wall

# Collect all .c files in the current directory
SRC = $(wildcard *.c)
# Remove the .c extension for output binaries
TARGETS = $(SRC:.c=)

all: $(TARGETS)

# Generic build rule
%: %.c
	$(CC) $(CFLAGS) $< -o $@

run:
	@echo "Usage: make <program_name> or ./<program_name>"

clean:
	rm -f $(TARGETS)



# Main Makefile for PI-CERTO project
# This Makefile builds and runs all projects in sequence:
# Compiler -> Assembler -> Executor

.PHONY: all clean compiler assembler executor run

# Default target builds all projects
all: compiler assembler executor

# Compiler project
compiler:
	@echo "Building compiler..."
	$(MAKE) -C compiler

# Assembler project
assembler:
	@echo "Building assembler..."
	$(MAKE) -C assembler

# Executor project
executor:
	@echo "Building executor..."
	$(MAKE) -C executor

# Clean all projects
clean:
	@echo "Cleaning all projects..."
	$(MAKE) -C compiler clean
	$(MAKE) -C assembler clean
	$(MAKE) -C executor clean
	@rm -f compiler/output/*.asm assembler/output/*.mem

# Run the complete pipeline with hardcoded filenames
run: all
	@echo "Running the full pipeline..."
	@if [ ! -f program.lpn ]; then \
		echo "Error: program.lpn not found in the current directory!"; \
		exit 1; \
	fi
	@echo "Step 1: Running compiler on program.lpn..."
	@mkdir -p compiler/output
	./compiler/bin/compilador program.lpn compiler/output/output.asm
	@echo "Step 2: Running assembler on compiler output..."
	@mkdir -p assembler/output
	./assembler/bin/assembler compiler/output/output.asm assembler/output/output.mem
	@echo "Step 3: Running executor with assembler output..."
	./executor/bin/executor assembler/output/output.mem
	@echo "Pipeline complete!"
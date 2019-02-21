# Default rule to execute when the make command has no arguments
all: producer	consumer

# Assembler Step: Produces the required object files.
consumer.o: consumer.c
	gcc -c consumer.c -o consumer.o -Wall -pedantic 

producer.o: producer.c
	gcc -c producer.c -o producer.o -Wall -pedantic 

# Linker Step: Produces the Final Executable File
consumer: consumer.o
	gcc consumer.o -o consumer -Wall -pedantic -pthread

producer: producer.o
	gcc producer.o -o producer -Wall -pedantic -pthread

# The cmd-line 'make clean' executes the following cmd
# Remover ALL files created during the previous steps.
clean: 
	rm -f consumer producer consumer.o producer.o

# Lists the "phony" rules in this file: 'all' and 'clean'
.PHONY: all clean

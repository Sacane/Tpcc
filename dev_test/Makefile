CC=gcc
CFLAGS=-Wall 
LDFLAGS = -lm -lMLV
OPT =-O2
OBJ = symbol.o symbols-table.o table_test.o
EXEC= table_test


all: $(EXEC)

table_test: $(OBJ)
		$(CC) -o $@ $^ 

table_test.o: table_test.c 

symbol.o: ../src/symbol.c ../src/symbol.h

symbols-table.o: ../src/symbols-table.c ../src/symbols-table.h

table-parser.o: ../src/table-parser.c ../src/table-parser.h

%.o: ../src/%.c
	$(CC) -c $< $(CFLAGS)


clean: 
	rm -f *.o

mrproper: clean
	
	rm -rf $(EXEC)

CC=gcc
DEPS= lex.yy
OBJ=$(DEPS:%=obj/%.o)
CFLAGS=-Wall -ly -lfl -pedantic -g 
EXEC=tpcas
PARSER=parser
LEXER=lexer

all : makedirs bin/$(EXEC)

makedirs:
	@mkdir -p bin
	@mkdir -p obj

bin/$(EXEC): obj/lex.yy.o  src/$(PARSER).tab.c 
	$(CC) -o $@ $^ src/tree.c $(LDFLAGS)

src/lex.yy.c: src/$(LEXER).lex src/tree.h src/$(PARSER).tab.h
	flex -o $@ src/$(LEXER).lex

src/$(PARSER).tab.c src/$(PARSER).tab.h: src/$(PARSER).y
	bison -d src/$(PARSER).y
	@mv $(PARSER).tab.c src/
	@mv $(PARSER).tab.h src/

obj/lex.yy.o: src/lex.yy.c
	$(CC) -c src/lex.yy.c -o obj/lex.yy.o $(CFLAGS)

obj/tree.o: src/tree.c src/tree.h
	$(CC) -c src/tree.c -o obj/tree.o $(CFLAGS)

clean:
	rm -f src/lex.yy.* src/$(PARSER).tab.* obj/* bin/*
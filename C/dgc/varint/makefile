SIMPLEX_PATH=/home/ben/Documents/simplex

SRC=main.c varint.c trackball.c
INC=varint.h trackball.h
LIBS=-lGL -lglut -lGLU -lGLEW -lsimplex -lcxsparse

varint: $(INC) $(SRC) simplex
	gcc -L $(SIMPLEX_PATH) $(SRC) $(LIBS) -o VarInt
	
debug: $(INC) $(SRC) simplex
	gcc -ggdb -L $(SIMPLEX_PATH) $(SRC) $(LIBS) -o VarDebug

test: $(INC) simplex
	gcc -L $(SIMPLEX_PATH) varint.c test.c $(LIBS) -o VarTest
	
simplex:
	cd $(SIMPLEX_PATH) && make

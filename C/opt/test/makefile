include ../../make.config
I=-I../
LIBS=-lcxsparse -lklu
OBJ=../line_search.o ../ls_opt.o ../utils.o

opt_test: main.c $(OBJ)
	$(CC) $(CFLAGS) $(I) $< -o $@ $(OBJ) $(LIBS)

$(OBJ):
	cd ../ && make

clean:
	cd ../ && make clean
	-rm opt_test

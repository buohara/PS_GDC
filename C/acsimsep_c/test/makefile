include ../../make.config
I=-I../../opt
LIBS=-lcxsparse -lklu
OPT=../../opt

OBJ= \
$(OPT)/ls_opt.o $(OPT)/line_search.o $(OPT)/utils.o

3bus_test: 3bus_test.c $(OBJ)
	$(CC) $(CFLAGS) $(I) $< -o $@ $(OBJ) $(LIBS)

$(OBJ):
	cd ../../opt && make
	
clean:
	-rm 3bus_test
	-rm *.o

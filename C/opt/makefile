include ../make.config
OBJ=line_search.o ls_opt.o utils.o

all: $(OBJ)

line_search.o: line_search.c opt.h
	$(CC) $(CFLAGS) -c $< -o $@
	
ls_opt.o: ls_opt.c opt.h
	$(CC) $(CFLAGS) -c $< -o $@
	
utils.o: utils.c opt.h
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	-rm *.o

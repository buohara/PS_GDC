include ../make.config
OBJ=ps.o load_outages.o fprint_outages.o

all: $(OBJ)

ps.o: ps.c ps.h
	$(CC) $(CFLAGS) -c $< -o $@
	
load_outages.o: load_outages.c ps.h
	$(CC) $(CFLAGS) -c $< -o $@
	
fprint_outages.o: fprint_outages.c ps.h
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	-rm *.o

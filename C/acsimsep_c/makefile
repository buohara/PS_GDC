include ../make.config
I=-I../ps -I../opt
LIBS=-lcxsparse -lklu

OBJ= \
../ps/ps.o ../ps/load_outages.o ../ps/fprint_outages.o \
get_ybus.o acpf.o

acsimsep_debug: main.c $(OBJ)
	$(CC) $(CFLAGS) $(I) $< -o $@ $(OBJ) $(LIBS)

../ps/ps.o:
	cd ../ps && make

acpf.o: acpf.c
	$(CC) $(CFLAGS) -c $(I) $< -o $@
	
get_ybus.o: get_ybus.c
	$(CC) $(CFLAGS) -c $(I) $< -o $@
	
clean:
	-rm acsimsep_debug *.o
	cd ../ps && make clean

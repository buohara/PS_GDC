include ../make.config
I=-I../ps
LIBS=-lcxsparse -lklu

OBJ= \
../ps/ps.o ../ps/load_outages.o ../ps/fprint_outages.o \
dcpf.o get_bbus.o redispatch.o \
update_relays.o dcsimsep.o

LIBGL=-lGL -lglut -lGLU -lGLEW

dcsimsep_debug: main.c $(OBJ)
	$(CC) $(CFLAGS) $(I) $< -o $@ $(OBJ) $(LIBS)
	
dcsimsep_gl: main_gl.c $(OBJ)
	$(CC) $(CFLAGS) $(I) $< -o $@ $(OBJ) $(LIBS) $(LIBGL)

../ps/ps.o:
	cd ../ps && make

dcpf.o: dcpf.c
	$(CC) $(CFLAGS) -c $(I) $< -o $@
	
get_bbus.o: get_bbus.c
	$(CC) $(CFLAGS) -c $(I) $< -o $@
	
redispatch.o: redispatch.c
	$(CC) $(CFLAGS) -c $(I) $< -o $@
	
update_relays.o: update_relays.c
	$(CC) $(CFLAGS) -c $(I) $< -o $@
	
dcsimsep.o: dcsimsep.c
	$(CC) $(CFLAGS) -c $(I) $< -o $@
	
clean:
	-rm dcsimsep_debug dcsimsep_gl *.o
	cd ../ps && make clean

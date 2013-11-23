simplex: simplex.c
	gcc -fPIC -c simplex.c -o simplex.o
	gcc -shared -Wl,-soname,libsimplex.so \
	-o libsimplex.so simplex.o -lm -lcxsparse

smptest: simplex.c test.c
	gcc simplex.c test.c -o smptest -lm -lcxsparse

clean:
	rm smptest

build: so-cpp
       
so-cpp: preprocessor.o
	gcc $^ -o $@

preprocessor.o: preprocessor.c
	gcc -Wall -c $^ -o $@

clean:
	rm -f *.o so-cpp

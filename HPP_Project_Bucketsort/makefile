parallel: main.c
	gcc -fopenmp main.c -O3 -lm -o bucketsort 
debug: main.c
	gcc -fopenmp main.c -lm -O0 -o bucketsort_debug -g
clean:
	rm -f bucketsort bucketsort_serial bucketsort_debug *.txt
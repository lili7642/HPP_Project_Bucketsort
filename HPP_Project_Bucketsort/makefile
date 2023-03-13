parallel: main.c
	gcc -fopenmp main.c -O3 -lm -o bucketsort 
serial: main.c
	gcc main.c -O3 -lm -o bucketsort_serial 
clean:
	rm -f bucketsort bucketsort_serial
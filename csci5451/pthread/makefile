calc_pi: calc_pi_thread.c calc_pi.c
	gcc calc_pi.c -o calc_without_thread -w
	gcc calc_pi_thread.c -o calc_with_thread -w
	./calc_without_thread -E
	./calc_with_thread -E
	rm calc_without_thread calc_with_thread


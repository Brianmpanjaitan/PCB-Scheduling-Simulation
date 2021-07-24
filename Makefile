pcb: pcb.o list.o
	gcc -g pcb.o list.o -o pcb
	
pcb.o: pcb.c
	gcc -c pcb.c
	
list.o: list.h

clean:
	rm *.o pcb

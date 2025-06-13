CC = gcc

OBJECTS = sample.o generic_list.o

all: sample clean

sample: $(OBJECTS)
	$(CC) $(OBJECTS) -o sample

sample.o: sample.c generic_list.h

generic_list.o: generic_list.c generic_list.h

clean:
	rm $(OBJECTS)

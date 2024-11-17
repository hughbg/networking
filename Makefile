CCOPT = -Wunused-variable

all: sender receiver 

sender: sender.c parse_args.o
	gcc $(CCOPT) -o sender sender.c parse_args.o

receiver: receiver.c parse_args.o
	gcc $(CCOPT) -o receiver receiver.c parse_args.o

parse_args.o: parse_args.h parse_args.c
	gcc $(CCOPT) -c parse_args.c
 
clean:
	rm sender receiver parse_args.o

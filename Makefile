CCOPT = -Wunused-variable

all: sender receiver vheader


sender: sender.c parse_args.o
	gcc $(CCOPT) -o sender sender.c parse_args.o

receiver: receiver.c parse_args.o vdif_lib.o
	gcc $(CCOPT) -o receiver receiver.c parse_args.o vdif_lib.o

vheader: vheader.c vdif_lib.o
	gcc $(CCOPT) -o vheader vheader.c vdif_lib.o

create_vdif: vheader.c vdif_lib.o
	gcc $(CCOPT) -o create_vdif create_vdif.c vdif_lib.o

parse_args.o: parse_args.h parse_args.c
	gcc $(CCOPT) -c parse_args.c

vdif_lib.o: vdif_lib.h vdif_lib.c
	gcc $(CCOPT) -c vdif_lib.c

clean:
	rm sender receiver parse_args.o vdif_lib.o

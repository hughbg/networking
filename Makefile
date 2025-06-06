COPT = -Wunused-variable
PROG = sender receiver vheader create_vdif spoof_sfxc

all: $(PROG)


sender: sender.c parse_args.o
	gcc $(COPT) -o sender sender.c parse_args.o

receiver: receiver.c parse_args.o vdif_lib.o
	gcc $(COPT) -o receiver receiver.c parse_args.o vdif_lib.o

vheader: vheader.c vdif_lib.o
	gcc $(COPT) -o vheader vheader.c vdif_lib.o

create_vdif: create_vdif.c vdif_lib.o
	gcc $(COPT) -o create_vdif create_vdif.c vdif_lib.o

spoof_sfxc: spoof_sfxc.c
	gcc $(COPT) spoof_sfxc.c -o spoof_sfxc

parse_args.o: parse_args.h parse_args.c
	gcc $(COPT) -c parse_args.c

vdif_lib.o: vdif_lib.h vdif_lib.c
	gcc $(COPT) -c vdif_lib.c

clean:
	rm $(PROG) *.o

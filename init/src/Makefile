#
####################

OPTIONS=-O2 -Wall -c

####################

default: all
all: init init_sv utmpset logon

####################

init: init.o log.a sig.a str.a wait.a
	gcc -s -o $@ $^
	#gcc -s -o $@ $^ -static

init_sv: init_sv.o log.a sig.a str.a wait.a
	gcc -s -o $@ $^

utmpset: utmpset.o log.a str.a
	gcc -s -o $@ $^

logon: logon.o log.a
	gcc -s -o $@ $^

####################
#%.o: %.c
#	gcc $(OPTIONS) -o $@ $<
####################

init.o: init.c
	gcc $(OPTIONS) -o $@ $<

init_sv.o: init_sv.c
	gcc $(OPTIONS) -o $@ $<

utmpset.o: utmpset.c
	gcc $(OPTIONS) -o $@ $<

logon.o: logon.c
	gcc $(OPTIONS) -o $@ $<

####################

log.o: log.c log.h
	gcc $(OPTIONS) -o $@ $<

log_errno.o: log_errno.c log.h
	gcc $(OPTIONS) -o $@ $<

sig_block.o: sig_block.c sig.h
	gcc $(OPTIONS) -o $@ $<

sig_catch.o: sig_catch.c sig.h
	gcc $(OPTIONS) -o $@ $<

str_chr.o: str.h str_chr.c
	gcc $(OPTIONS) -o $@ str_chr.c

str_equal.o: str.h str_equal.c
	gcc $(OPTIONS) -o $@ str_equal.c

str_len.o: str.h str_len.c
	gcc $(OPTIONS) -o $@ str_len.c

wait_nohang.o: wait_nohang.c
	gcc $(OPTIONS) -o $@ $<

wait_pid.o: wait_pid.c
	gcc $(OPTIONS) -o $@ $<

####################

log.a: log.o
	rm -f log.a
	ar cr log.a log.o
	ranlib log.a

str.a: str_equal.o str_len.o
	rm -f str.a
	ar cr str.a str_equal.o str_len.o
	ranlib str.a

sig.a: sig_block.o sig_catch.o
	rm -f sig.a
	ar cr sig.a sig_block.o sig_catch.o
	ranlib sig.a

wait.a: wait_nohang.o wait_pid.o
	rm -f wait.a
	ar cr wait.a wait_nohang.o wait_pid.o
	ranlib wait.a

####################

clean:
	rm -f *.o
	rm -f *.a
	rm -f *~

cleaner: clean
	rm -f init init_sv utmpset logon

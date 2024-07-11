#
####################

OPTIONS=-O2 -Wall -c

####################

default: init runsv runsvdir utmpset

####################

init: init.o log.a sig.a str.a wait.a
	gcc -s -o $@ $^
	#gcc -s -o $@ $^ -static

#runsv: runsv.o unix.a byte.a time.a
runsv: runsv.o log.a sig.a str.a time.a wait.a
	gcc -s -o $@ $^

#runsvdir: runsvdir.o unix.a byte.a time.a
runsvdir: runsvdir.o log.a sig.a str.a time.a wait.a
	gcc -s -o $@ $^

#utmpset: utmpset.o unix.a byte.a
utmpset: utmpset.o log.a str.a
	gcc -s -o $@ $^

####################
#%.o: %.c
#	gcc $(OPTIONS) -o $@ $<
####################

init.o: init.c
	gcc $(OPTIONS) -o $@ $<

runsv.o: runsv.c
	gcc $(OPTIONS) -o $@ $<

runsvdir.o: runsvdir.c
	gcc $(OPTIONS) -o $@ $<

utmpset.o: utmpset.c
	gcc $(OPTIONS) -o $@ $<


pmatch.o: pmatch.c
	gcc $(OPTIONS) -o $@ $<

fmt_ptime.o: fmt_ptime.c
	gcc $(OPTIONS) -o $@ $<


####################

iopause.o: iopause.c iopause.h tai.h taia.h uint64.h
	gcc $(OPTIONS) -o $@ $<

log.o: log.c log.h
	gcc $(OPTIONS) -o $@ $<

log_errno.o: log_errno.c log.h
	gcc $(OPTIONS) -o $@ $<

sig_block.o: sig_block.c sig.h
	gcc $(OPTIONS) -o $@ $<

sig_catch.o: sig_catch.c sig.h
	gcc $(OPTIONS) -o $@ $<

sig_pause.o: sig_pause.c sig.h
	gcc $(OPTIONS) -o $@ $<

str_chr.o: str.h str_chr.c
	gcc $(OPTIONS) -o $@ str_chr.c

str_diff.o: str.h str_diff.c
	gcc $(OPTIONS) -o $@ str_diff.c

str_equal.o: str.h str_equal.c
	gcc $(OPTIONS) -o $@ str_equal.c

str_len.o: str.h str_len.c
	gcc $(OPTIONS) -o $@ str_len.c

str_start.o: str.h str_start.c
	gcc $(OPTIONS) -o $@ str_start.c

tai_now.o: tai.h tai_now.c uint64.h
	gcc $(OPTIONS) -o $@ tai_now.c

tai_pack.o: tai.h tai_pack.c uint64.h
	gcc $(OPTIONS) -o $@ tai_pack.c

tai_sub.o: tai.h tai_sub.c uint64.h
	gcc $(OPTIONS) -o $@ tai_sub.c

tai_unpack.o: tai.h tai_unpack.c uint64.h
	gcc $(OPTIONS) -o $@ tai_unpack.c

taia_add.o: tai.h taia.h taia_add.c uint64.h
	gcc $(OPTIONS) -o $@ taia_add.c

taia_approx.o: tai.h taia.h taia_approx.c uint64.h
	gcc $(OPTIONS) -o $@ taia_approx.c

taia_frac.o: tai.h taia.h taia_frac.c uint64.h
	gcc $(OPTIONS) -o $@ taia_frac.c

taia_less.o: tai.h taia.h taia_less.c uint64.h
	gcc $(OPTIONS) -o $@ taia_less.c

taia_now.o: tai.h taia.h taia_now.c uint64.h
	gcc $(OPTIONS) -o $@ taia_now.c

taia_pack.o: tai.h taia.h taia_pack.c uint64.h
	gcc $(OPTIONS) -o $@ taia_pack.c

taia_sub.o: tai.h taia.h taia_sub.c uint64.h
	gcc $(OPTIONS) -o $@ taia_sub.c

taia_uint.o: tai.h taia.h taia_uint.c uint64.h
	gcc $(OPTIONS) -o $@ taia_uint.c

wait_nohang.o: wait_nohang.c
	gcc $(OPTIONS) -o $@ $<

wait_pid.o: wait_pid.c
	gcc $(OPTIONS) -o $@ $<

####################

byte.a: byte_chr.o byte_copy.o byte_cr.o byte_diff.o byte_rchr.o \
fmt_uint.o fmt_uint0.o fmt_ulong.o \
str_chr.o str_diff.o str_len.o str_start.o
	rm -f byte.a
	ar cr byte.a byte_chr.o byte_copy.o byte_cr.o byte_diff.o \
	byte_rchr.o fmt_uint.o fmt_uint0.o fmt_ulong.o \
	str_chr.o str_diff.o str_len.o str_start.o
	ranlib byte.a

log.a: log.o
	rm -f log.a
	ar cr log.a log.o
	ranlib log.a

str.a: str_equal.o str_len.o
	rm -f str.a
	ar cr str.a str_equal.o str_len.o
	ranlib str.a

sig.a: sig_block.o sig_catch.o sig_pause.o
	rm -f sig.a
	ar cr sig.a sig_block.o sig_catch.o sig_pause.o
	ranlib sig.a

time.a: iopause.o tai_now.o tai_pack.o tai_sub.o tai_unpack.o \
taia_add.o taia_approx.o taia_frac.o taia_less.o taia_now.o \
taia_pack.o taia_sub.o taia_uint.o
	rm -f time.a
	ar cr time.a iopause.o tai_now.o tai_pack.o tai_sub.o \
	tai_unpack.o taia_add.o taia_approx.o taia_frac.o taia_less.o \
	taia_now.o taia_pack.o taia_sub.o taia_uint.o
	ranlib time.a

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
	rm -f init runsv runsvdir utmpset

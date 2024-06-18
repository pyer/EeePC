#
####################

OPTIONS=-O2 -Wall -c

####################

default: chpst init runsv runsvdir utmpset

####################

init: init.o unix.a byte.a
	gcc -s -o $@ $^ -static
	cp $@ ../

runsv: runsv.o unix.a byte.a time.a
	gcc -s -o $@ $^
	cp $@ ../

runsvdir: runsvdir.o unix.a byte.a time.a
	gcc -s -o $@ $^
	cp $@ ../

utmpset: utmpset.o unix.a byte.a
	gcc -s -o $@ $^
	cp $@ ../

chpst: chpst.o uidgid.o unix.a byte.a
	gcc -s -o $@ $^
	cp $@ ../

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

chpst.o: chpst.c
	gcc $(OPTIONS) -o $@ $<


uidgid.o: uidgid.c uidgid.h
	gcc $(OPTIONS) -o $@ $<

pmatch.o: pmatch.c
	gcc $(OPTIONS) -o $@ $<

fmt_ptime.o: fmt_ptime.c
	gcc $(OPTIONS) -o $@ $<


####################

alloc.o: alloc.c alloc.h error.h
	gcc $(OPTIONS) -o $@ $<

alloc_re.o: alloc_re.c alloc.h byte.h
	gcc $(OPTIONS) -o $@ $<

buffer.o: buffer.c buffer.h
	gcc $(OPTIONS) -o $@ $<

buffer_0.o: buffer_0.c buffer.h
	gcc $(OPTIONS) -o $@ $<

buffer_1.o: buffer_1.c buffer.h
	gcc $(OPTIONS) -o $@ $<

buffer_2.o: buffer_2.c buffer.h
	gcc $(OPTIONS) -o $@ $<

buffer_get.o: buffer_get.c buffer.h byte.h error.h
	gcc $(OPTIONS) -o $@ $<

buffer_put.o: buffer_put.c buffer.h byte.h error.h str.h
	gcc $(OPTIONS) -o $@ $<

buffer_read.o: buffer_read.c buffer.h
	gcc $(OPTIONS) -o $@ $<

buffer_write.o: buffer_write.c buffer.h
	gcc $(OPTIONS) -o $@ $<

byte_chr.o: byte_chr.c byte.h
	gcc $(OPTIONS) -o $@ $<

byte_copy.o: byte_copy.c byte.h
	gcc $(OPTIONS) -o $@ $<

byte_cr.o: byte_cr.c byte.h
	gcc $(OPTIONS) -o $@ $<

byte_diff.o: byte_diff.c byte.h
	gcc $(OPTIONS) -o $@ $<

byte_rchr.o: byte_rchr.c byte.h
	gcc $(OPTIONS) -o $@ $<

coe.o: coe.c coe.h
	gcc $(OPTIONS) -o $@ $<

env.o: env.c env.h str.h
	gcc $(OPTIONS) -o $@ $<

error.o: error.c error.h
	gcc $(OPTIONS) -o $@ $<

error_str.o: error_str.c error.h
	gcc $(OPTIONS) -o $@ $<

fd_copy.o: fd_copy.c fd.h
	gcc $(OPTIONS) -o $@ $<

fd_move.o: fd_move.c fd.h
	gcc $(OPTIONS) -o $@ $<

fifo.o: fifo.c fifo.h hasmkffo.h
	gcc $(OPTIONS) -o $@ $<

fmt_uint.o: fmt_uint.c fmt.h
	gcc $(OPTIONS) -o $@ $<

fmt_uint0.o: fmt_uint0.c fmt.h
	gcc $(OPTIONS) -o $@ $<

fmt_ulong.o: fmt_ulong.c fmt.h
	gcc $(OPTIONS) -o $@ $<

iopause.o: iopause.c iopause.h select.h tai.h taia.h uint64.h
	gcc $(OPTIONS) -o $@ $<

lock_ex.o: lock_ex.c hasflock.h lock.h
	gcc $(OPTIONS) -o $@ $<

lock_exnb.o: lock_exnb.c hasflock.h lock.h
	gcc $(OPTIONS) -o $@ $<

ndelay_off.o: ndelay_off.c ndelay.h
	gcc $(OPTIONS) -o $@ $<

ndelay_on.o: ndelay_on.c ndelay.h
	gcc $(OPTIONS) -o $@ $<

open_append.o: open.h open_append.c
	gcc $(OPTIONS) -o $@ open_append.c

open_read.o: open.h open_read.c
	gcc $(OPTIONS) -o $@ open_read.c

open_trunc.o: open.h open_trunc.c
	gcc $(OPTIONS) -o $@ open_trunc.c

open_write.o: open.h open_write.c
	gcc $(OPTIONS) -o $@ open_write.c

openreadclose.o: error.h gen_alloc.h open.h openreadclose.c \
openreadclose.h readclose.h stralloc.h
	gcc $(OPTIONS) -o $@ openreadclose.c

pathexec_env.o: alloc.h byte.h env.h gen_alloc.h pathexec.h \
pathexec_env.c str.h stralloc.h
	gcc $(OPTIONS) -o $@ pathexec_env.c

pathexec_run.o: env.h error.h gen_alloc.h pathexec.h \
pathexec_run.c str.h stralloc.h
	gcc $(OPTIONS) -o $@ pathexec_run.c

readclose.o: error.h gen_alloc.h readclose.c readclose.h \
stralloc.h
	gcc $(OPTIONS) -o $@ readclose.c

scan_ulong.o: scan_ulong.c scan.h
	gcc $(OPTIONS) -o $@ $<

seek_set.o: seek_set.c seek.h
	gcc $(OPTIONS) -o $@ $<

sgetopt.o: sgetopt.c sgetopt.h subgetopt.h buffer.h
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

str_len.o: str.h str_len.c
	gcc $(OPTIONS) -o $@ str_len.c

str_start.o: str.h str_start.c
	gcc $(OPTIONS) -o $@ str_start.c

stralloc_cat.o: byte.h gen_alloc.h stralloc.h stralloc_cat.c
	gcc $(OPTIONS) -o $@ stralloc_cat.c

stralloc_catb.o: byte.h gen_alloc.h stralloc.h \
stralloc_catb.c
	gcc $(OPTIONS) -o $@ stralloc_catb.c

stralloc_cats.o: byte.h gen_alloc.h str.h stralloc.h \
stralloc_cats.c
	gcc $(OPTIONS) -o $@ stralloc_cats.c

stralloc_eady.o: alloc.h gen_alloc.h gen_allocdefs.h \
stralloc.h stralloc_eady.c
	gcc $(OPTIONS) -o $@ stralloc_eady.c

stralloc_opyb.o: byte.h gen_alloc.h stralloc.h \
stralloc_opyb.c
	gcc $(OPTIONS) -o $@ stralloc_opyb.c

stralloc_opys.o: byte.h gen_alloc.h str.h stralloc.h \
stralloc_opys.c
	gcc $(OPTIONS) -o $@ stralloc_opys.c

stralloc_pend.o: alloc.h gen_alloc.h gen_allocdefs.h \
stralloc.h stralloc_pend.c
	gcc $(OPTIONS) -o $@ stralloc_pend.c

strerr_die.o: buffer.h strerr.h strerr_die.c
	gcc $(OPTIONS) -o $@ strerr_die.c

strerr_sys.o: error.h strerr.h strerr_sys.c
	gcc $(OPTIONS) -o $@ strerr_sys.c

subgetopt.o: subgetopt.c subgetopt.h
	gcc $(OPTIONS) -o $@ $<

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

wait_nohang.o: wait_nohang.c haswaitp.h
	gcc $(OPTIONS) -o $@ $<

wait_pid.o: wait_pid.c error.h haswaitp.h
	gcc $(OPTIONS) -o $@ $<

####################

byte.a: byte_chr.o byte_copy.o byte_cr.o byte_diff.o byte_rchr.o \
fmt_uint.o fmt_uint0.o fmt_ulong.o scan_ulong.o str_chr.o \
str_diff.o str_len.o str_start.o
	rm -f byte.a
	ar cr byte.a byte_chr.o byte_copy.o byte_cr.o byte_diff.o \
	byte_rchr.o fmt_uint.o fmt_uint0.o fmt_ulong.o scan_ulong.o str_chr.o \
	str_diff.o str_len.o str_start.o
	ranlib byte.a

time.a: iopause.o tai_now.o tai_pack.o tai_sub.o tai_unpack.o \
taia_add.o taia_approx.o taia_frac.o taia_less.o taia_now.o \
taia_pack.o taia_sub.o taia_uint.o
	rm -f time.a
	ar cr time.a iopause.o tai_now.o tai_pack.o tai_sub.o \
	tai_unpack.o taia_add.o taia_approx.o taia_frac.o taia_less.o \
	taia_now.o taia_pack.o taia_sub.o taia_uint.o
	ranlib time.a

unix.a: alloc.o alloc_re.o buffer.o buffer_0.o buffer_1.o buffer_2.o \
buffer_get.o buffer_put.o buffer_read.o buffer_write.o coe.o env.o \
error.o error_str.o fd_copy.o fd_move.o fifo.o lock_ex.o lock_exnb.o \
ndelay_off.o ndelay_on.o open_append.o open_read.o \
open_trunc.o open_write.o openreadclose.o pathexec_env.o \
pathexec_run.o readclose.o seek_set.o sgetopt.o \
sig_block.o sig_catch.o sig_pause.o stralloc_cat.o stralloc_catb.o \
stralloc_cats.o stralloc_eady.o stralloc_opyb.o stralloc_opys.o \
stralloc_pend.o strerr_die.o strerr_sys.o subgetopt.o wait_nohang.o \
wait_pid.o
	rm -f unix.a
	ar cr unix.a alloc.o alloc_re.o buffer.o buffer_0.o buffer_1.o buffer_2.o \
	buffer_get.o buffer_put.o buffer_read.o buffer_write.o coe.o env.o \
	error.o error_str.o fd_copy.o fd_move.o fifo.o lock_ex.o lock_exnb.o \
	ndelay_off.o ndelay_on.o open_append.o open_read.o \
	open_trunc.o open_write.o openreadclose.o pathexec_env.o \
	pathexec_run.o readclose.o seek_set.o sgetopt.o \
	sig_block.o sig_catch.o sig_pause.o stralloc_cat.o stralloc_catb.o \
	stralloc_cats.o stralloc_eady.o stralloc_opyb.o stralloc_opys.o \
	stralloc_pend.o strerr_die.o strerr_sys.o subgetopt.o wait_nohang.o \
	wait_pid.o
	ranlib unix.a

####################

clean:
	rm -f *.o
	rm -f *.a
	rm -f *~


#
####################

OPTIONS=-O2 -Wall -c

####################

default: log_test str_test

####################

log_test: log_test.o ../src/log.a
	gcc -s -o $@ $< ../src/log.a
	./$@ >stdout.result 2>stderr.result
	@diff expected_stdout stdout.result
	@diff expected_stderr stderr.result
	@rm -f $@ $< *.result

str_test: str_test.o ../src/str.a
	gcc -s -o $@ $< ../src/str.a
	./$@

speed_test: speed_test.o
	gcc -s -o $@ $<
	./$@

####################
%.o: %.c
	gcc $(OPTIONS) -o $@ $<
####################

clean:
	rm -f *.o
	rm -f *.a
	rm -f *~

####################

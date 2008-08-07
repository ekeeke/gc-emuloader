all:	lzmaadd lzmaex

lzmaadd:	lzmaadd.c
	@gcc -O2 $^ -o $@
	@strip $@

lzmaex:		lzmaex.c
	@gcc -O2 $^ -o $@
	@strip $@

clean:
	@rm lzmaadd lzmaex

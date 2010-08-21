clean:
	rm -f *.o *.lo *.slo
	rm -f *.la
	rm -rf .libs

mod_sotarok.la:
	apxs -c -o mod_sotarok.so mod_sotarok.c

mod_mikko.la:
	apxs -c -o mod_mikko.so mod_mikko.c

.PHONY: clean

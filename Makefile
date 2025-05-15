install:
	clib install --dev

test:
	@$(CC) test.c $(CFLAGS) -I src -I deps $(LDFLAGS) -o $@
	@./$@

.PHONY: test

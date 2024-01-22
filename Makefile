
test:
	clib install --dev
	@$(CC) test.c src/file_utils.c -std=c99 -I src -I deps -o $@
	@./$@

.PHONY: test

T = $(wildcard t[0-9][0-9][0-9][0-9]*.sh)

test: $(T)

slow:
	LYEKKA_TEST_OPTS=--slow $(MAKE)

$(T):
	@echo "*** $@ ***"; ./$@ $(LYEKKA_TEST_OPTS)

pre-clean:
	$(RM) -r test-results

clean:
	$(RM) -r 'trashdir'.* test-results

.PHONY: pre-clean $(T) test

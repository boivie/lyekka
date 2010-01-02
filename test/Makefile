T = $(wildcard t[0-9][0-9][0-9][0-9]*.sh)

test: $(T)

$(T):
	@echo "*** $@ ***"; ./$@ 

pre-clean:
	$(RM) -r test-results

clean:
	$(RM) -r 'trashdir'.* test-results

.PHONY: pre-clean $(T) test

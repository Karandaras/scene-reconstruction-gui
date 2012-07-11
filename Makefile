DIRS = src

compile:
			for i in $(DIRS); do make -C $$i; done

.PHONY: clean
clean:
			for i in $(DIRS); do make -C $$i clean; done

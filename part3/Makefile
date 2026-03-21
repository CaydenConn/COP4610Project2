all:
	$(MAKE) -C src
	$(MAKE) -C tests/elevator-test
	$(MAKE) -C tests/system-calls-test syscheck.ko

clean:
	$(MAKE) -C src clean
	$(MAKE) -C tests/elevator-test clean
	$(MAKE) -C tests/system-calls-test clean

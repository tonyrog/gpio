all:
	(cd c_src && $(MAKE) all)
	(cd src && $(MAKE) all)
	(cd test && $(MAKE) all)

clean:
	(cd test && $(MAKE) clean)
	(cd src && $(MAKE) clean)
	(cd c_src && $(MAKE) all)


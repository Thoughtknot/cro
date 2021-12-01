make:
	cd compiler && $(MAKE) all
	cd runtime && $(MAKE) all

clean:
	cd compiler && $(MAKE) clean
	cd runtime && $(MAKE) clean
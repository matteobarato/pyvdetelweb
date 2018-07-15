cleanpicotcp:
	(cd pycoclib/picotcp && make clean)

clean: cleanpicotcp
	rm pycoclib/include || echo "No need to clean pycoclib/include..."
	rm pycoclib/lib || echo "No need to clean pycoclib/lib..."
	rm pycoclib/modules || echo "No need to clean pycoclib/modules..."

picotcp: clean
	(cd pycoclib/picotcp && make posix core ARCH=shared && make test ARCH=shared)
	(cd pycoclib && ln -s picotcp/build/include ./include)
	(cd pycoclib && ln -s picotcp/build/lib ./lib)
	(cd pycoclib && ln -s picotcp/build/modules ./modules)

cffi: picotcp
	(cd pycotcp && python fdpicobuild.py) || echo "FDPICOTCP NOT FOUND IN THE SYSTEM, -l fdpicotcp AND FDPicoTCPAdapter WILL NOT BE AVAILABLE"
	(cd pycotcp && python lwipv6build.py) || echo "LWIPV6 NOT FOUND IN THE SYSTEM, -l lwipv6 AND Lwipv6Adapter WILL NOT BE AVAILABLE"

install: cffi
	python setup.py install --user

globalinstall: cffi
	sudo python setup.py install

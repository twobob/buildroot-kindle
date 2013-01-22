EXTRA_DIST = Doxyfile.in

all: stamp-doxygen

stamp-doxygen:
if HAVE_DOXYGEN
	doxygen Doxyfile
	touch stamp-doxygen
endif

clean: clean-local
clean-local:
	-rm -Rf html
	-rm -f stamp-doxygen
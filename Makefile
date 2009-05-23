PREFIX = /opt/local/gentoo
# glib_inc = `pkg-config glib-2.0 --cflags`
# glib_dir = `pkg-config glib-2.0 --libs`
glib_cflags = -I/opt/local/gentoo/usr/include/glib-2.0 -I/opt/local/gentoo/usr/lib/glib-2.0/include
glib_libs = -L/opt/local/gentoo/usr/lib #-lglib-2.0 -lintl
irssi_dir = ${PREFIX}/usr/include/irssi
gmp_libs = ${PREFIX}/usr/lib/libgmp.a

# *** to find glib directories, try:
# >>> pkg-config --cflags glib-2.0
# >>> pkg-config --cflags glib
# >>> locate glib.h
# >>> locate glibconfig.h

#glib_inc = /usr/local/include/glib-1.2
#glib_dir = /usr/local/lib/glib-1.2
#irssi_dir = $(HOME)/irssi-0.8.12

all_include_dirs = -I. $(glib_cflags) $(glib_libs) $(gpm_libs) -I$(irssi_dir) -I$(irssi_dir)/src -I$(irssi_dir)/src/core -I$(irssi_dir)/src/fe-common/core -I$(irssi_dir)/src/irc/core


# maybe you have to use gmake instead of make, define it here:
MAKE=@make


CC=gcc
# CCFLAGS=-fPIC -O2
# Eat this! gcc4/MacOSX # CCFLAGS="-fPIC -Wno-pointer-sign"


all:	note

	$(MAKE) misc
	$(MAKE) DH
	$(MAKE) FiSH

	$(MAKE) finished


note:
	@echo "Press ENTER to continue or CTRL+C to abort..."; read junk

	$(MAKE) -s clean
	@echo "Compiling now..."; echo


clean:
	@echo; echo Cleaning up...; echo
	@rm -f SHA-256.o
	@rm -f base64.o
	@rm -f blowfish.o
	@rm -f cfgopts.o
	@rm -f DH1080.o
	@rm -f FiSH.o
	@rm -f libfish.so
	@rm -f DH1080.so


DH:
	$(CC) $(CCFLAGS) -static -c DH1080.c
	$(CC) $(CCFLAGS) -shared DH1080.o SHA-256.o base64.o -o DH1080.so
	@echo


misc:
	$(CC) $(CCFLAGS) -c -static SHA-256.c
	$(CC) $(CCFLAGS) -c -static base64.c
	$(CC) $(CCFLAGS) -c -static blowfish.c
	$(CC) $(CCFLAGS) -c -static cfgopts.c
	@echo


FiSH:
	$(CC) $(CCFLAGS) $(all_include_dirs) -static -Wall -c FiSH.c
	$(CC) $(CCFLAGS) -shared FiSH.o SHA-256.o base64.o blowfish.o cfgopts.o DH1080.o -o libfish.so
	@echo


mac:	macosx
macosx:	note

	@# Eat this! gcc4/MacOSX (maybe you'll also need -fPIC)
	@# CCFLAGS="-fPIC -Wno-pointer-sign"
	@#CCFLAGS='$(CCFLAGS) -fno-common'

	$(CC) $(CCFLAGS) -c SHA-256.c
	$(CC) $(CCFLAGS) -c base64.c
	$(CC) $(CCFLAGS) -c blowfish.c
	$(CC) $(CCFLAGS) -c cfgopts.c
	$(CC) $(CCFLAGS) -c DH1080.c
	@echo

	$(CC) $(CCFLAGS) $(all_include_dirs) -Wall -c FiSH.c
	$(CC) $(CCFLAGS) $(gmp_libs) -bundle -flat_namespace -undefined suppress FiSH.o SHA-256.o base64.o blowfish.o cfgopts.o DH1080.o  -o libfish.bundle
	# $(CC) $(CCFLAGS) $(gmp_libs) -bundle FiSH.o SHA-256.o base64.o blowfish.o cfgopts.o DH1080.o  -o libfish.bundle

	$(MAKE) finished



amd64:  note

	@if [ ! -f "mirdef_amd64.h" ] && [ ! -f "MIRACL/mirdef_amd64.h" ]; then echo "WARN: mirdef_amd64.h not found! :( Press ENTER to continue or CTRL+C to abort..."; read junk; echo; fi
	@if [ -f "mirdef_amd64.h" ]; then echo "Okay: Using $(PWD)/mirdef_amd64.h ..."; cp mirdef_amd64.h mirdef.h; echo; fi
	@if [ ! -f "mirdef_amd64.h" ]; then echo "Okay: Using $(PWD)/MIRACL/mirdef_amd64.h ..."; cp MIRACL/mirdef_amd64.h mirdef.h; echo; fi

	$(MAKE) misc
	$(MAKE) DH
	$(MAKE) FiSH

	$(MAKE) finished



linux64: x64
x64:    note

	@if [ ! -f "mirdef.h64" ] && [ ! -f "MIRACL/mirdef.h64" ]; then echo "WARN: mirdef.h64 not found! :( Press ENTER to continue or CTRL+C to abort..."; read junk; echo; fi
	@if [ -f "mirdef.h64" ]; then echo "Okay: Using $(PWD)/mirdef.h64 ..."; cp mirdef.h64 mirdef.h; echo; fi
	@if [ ! -f "mirdef.h64" ]; then echo "Okay: Using $(PWD)/MIRACL/mirdef.h64 ..."; cp MIRACL/mirdef.h64 mirdef.h; echo; fi

	$(MAKE) misc
	$(MAKE) DH
	$(MAKE) FiSH

	$(MAKE) finished



sparc32: note

	@if [ ! -f "mirdef_sparc32.h" ] && [ ! -f "MIRACL/mirdef_sparc32.h" ]; then echo "WARN: mirdef_sparc32.h not found! :( Press ENTER to continue or CTRL+C to abort..."; read junk; echo; fi
	@if [ -f "mirdef_sparc32.h" ]; then echo "Okay: Using $(PWD)/mirdef_sparc32.h ..."; cp mirdef_sparc32.h mirdef.h; echo; fi
	@if [ ! -f "mirdef_sparc32.h" ]; then echo "Okay: Using $(PWD)/MIRACL/mirdef_sparc32.h ..."; cp MIRACL/mirdef_sparc32.h mirdef.h; echo; fi

	@CCFLAGS="$(CCFLAGS) -m32"

	$(MAKE) misc
	$(MAKE) DH
	$(MAKE) FiSH

	$(MAKE) finished



sparc64: note

	@if [ ! -f "mirdef_sparc64.h" ] && [ ! -f "MIRACL/mirdef_sparc64.h" ]; then echo "WARN: mirdef_sparc64.h not found! :( Press ENTER to continue or CTRL+C to abort..."; read junk; echo; fi
	@if [ -f "mirdef_sparc64.h" ]; then echo "Okay: Using $(PWD)/mirdef_sparc64.h ..."; cp mirdef_sparc64.h mirdef.h; echo; fi
	@if [ ! -f "mirdef_sparc64.h" ]; then echo "Okay: Using $(PWD)/MIRACL/mirdef_sparc64.h ..."; cp MIRACL/mirdef_sparc64.h mirdef.h; echo; fi

	@CCFLAGS="$(CCFLAGS) -m64"

	$(MAKE) misc
	$(MAKE) DH
	$(MAKE) FiSH

	$(MAKE) finished



finished:
	@if [ ! -f "libfish.so" ]; then echo "ERROR: compile failed :("; echo; exit 1; fi
	@echo 
	@ls -l libfish.so
	@echo 
	@echo -e "Compiling finished -->> $(PWD)/\033[1mlibfish.so\033[0m"
	@echo
	@echo -e "You might want to: \033[1mcp libfish.so ~/.irssi/modules/\033[0m -or- \033[1mcp libfish.so /usr/local/lib/irssi/modules/\033[0m"
	@echo

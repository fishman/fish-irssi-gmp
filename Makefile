glib_inc = /usr/local/include/glib-2.0
glib_dir = /usr/local/lib/glib-2.0
irssi_dir = /usr/local/include/irssi

# *** to find glib directories, try:
# >>> pkg-config --cflags glib-2.0
# >>> pkg-config --cflags glib
# >>> locate glib.h
# >>> locate glibconfig.h

#glib_inc = /usr/local/include/glib-1.2
#glib_dir = /usr/local/lib/glib-1.2
#irssi_dir = $(HOME)/irssi-0.8.12

all_include_dirs = -I. -I$(glib_inc) -I$(glib_inc)/include -I$(glib_inc)/glib -I$(glib_dir) -I$(glib_dir)/include -I$(glib_dir)/glib -I$(irssi_dir) -I$(irssi_dir)/src -I$(irssi_dir)/src/core -I$(irssi_dir)/src/fe-common/core -I$(irssi_dir)/src/irc/core


# maybe you have to use gmake instead of make, define it here:
MAKE=@make


CC=gcc
CCFLAGS=-fPIC -O2
# Eat this! gcc4/MacOSX # CCFLAGS="-fPIC -Wno-pointer-sign"


all:	note

	@if [ ! -f "mirdef.lnx" ] && [ ! -f "MIRACL/mirdef.lnx" ]; then echo "WARN: mirdef.lnx not found! :( Press ENTER to continue or CTRL+C to abort..."; read junk; echo; fi
	@if [ -f "mirdef.lnx" ]; then echo "Okay: Using $(pwd)/mirdef.lnx ..."; cp mirdef.lnx mirdef.h; echo; fi
	@if [ ! -f "mirdef.lnx" ]; then echo "Okay: Using $(pwd)/MIRACL/mirdef.lnx ..."; cp MIRACL/mirdef.lnx mirdef.h; echo; fi

	$(MAKE) misc
	$(MAKE) DH
	$(MAKE) FiSH

	$(MAKE) finished


note:
	@echo
	@echo -e "######################## \033[7m\033[1mFiSH for irssi COMPILING NOTES\033[0m ########################"
	@echo "--------------------------------------------------------------------------------"
	@echo    "You'll need MIRACL (http://www.shamus.ie/) to compile FiSH! It is suggested"
	@echo    "to recompile miracl.a on your target machine (compile instructions can be found"
	@echo -e "in miracl.zip/\033[1mlinux.txt\033[0m). Also, make sure you are using the correct mirdef.h"
	@echo -e "(especially in case of exotic systems). On \033[1mamd64\033[0m systems use 'mirdef_amd64.h'"
	@echo -e "and have a look at miracl.zip/amd64.txt, use 'mirdef_macosx.h' for \033[1mMacOSX\033[0m. You"
	@echo -e "can test MIRACL with \033[1mpk-demo\033[0m."
	@echo
	@echo -e "Using irssi source from: \033[1m$(irssi_dir)\033[0m"
	@echo -e "Using glib.h from: \033[1m$(glib_dir)\033[0m"
	@echo -e "Using glibconfig.h from: \033[1m$(glib_inc)\033[0m"
	@echo    "-- Please make sure you have all of them installed!"
	@echo    "   (try 'pkg-config --cflags glib-2.0' or 'pkg-config --cflags glib'"
	@echo    "    or 'locate glib.h' and 'locate glibconfig.h')"
	@echo
	@echo -e "-- If you plan to use a \033[4mcustom\033[0m password for your blow.ini (via /setinipw),"
	@echo -e "-- you can \033[4mskip\033[0m the following note!"
	@echo    "You are adviced to change default_iniKey in FiSH.h - But remember, changing the"
	@echo    "default blow.ini password will make your old keys in blow.ini unusable!"
	@echo
	@echo -e "\033[1m>> \033[4mUsual procedure to compile FiSH:\033[0m"
	@echo    "   unzip FiSH-irssi.v1.00-source.zip; cd FiSH-irssi.v1.00-source"
	@echo    "   mkdir MIRACL;cd MIRACL; cp ../mir_amd64 amd64; cp ../mir_macosx macosx"
	@echo    "   cp ../mir_sparc32 sparc32; cp ../mir_sparc64 sparc64"
	@echo    "   wget ftp://ftp.computing.dcu.ie/pub/crypto/miracl.zip  (or 'curl -O' instead of wget)"
	@echo -e "   \033[4munzip -j -aa -L miracl.zip\033[0m  (see linux.txt/amd64.txt for more info)"
	@echo -e "   \033[4mbash linux\033[0m  (or '\033[4mbash linux64\033[0m' / '\033[4mbash amd64\033[0m' / '\033[4mbash macosx\033[0m' / '\033[4mbash sparc32/64\033[0m')"
	@echo    "   cp miracl.a ../;cd .."
	@echo    "   <adjust glib_dir/glib_inc and irssi_dir in 'Makefile'>"
	@echo -e "   \033[4mmake\033[0m  (or '\033[4mmake macosx\033[0m' / '\033[4mmake linux64\033[0m' / '\033[4mmake amd64\033[0m'  / '\033[4mmake sparc32/64\033[0m')"
	@echo    "   cp libfish.so ~/.irssi/modules; cp libfish.so /usr/local/lib/irssi/modules/"
	@echo    "--------------------------------------------------------------------------------"
	@echo
	@echo "Press ENTER to continue or CTRL+C to abort..."; read junk

	@if [ ! -f "miracl.a" ] && [ ! -f "MIRACL/miracl.a" ]; then echo "ERROR: miracl.a not found! You didn't compile MIRACL, did you?"; echo; exit 1; fi
	@if [ -f "MIRACL/miracl.a" ]; then echo "Okay: Using $(PWD)/MIRACL/miracl.a ..."; cp MIRACL/miracl.a miracl.a; echo; fi
	@if [ ! -f "MIRACL/miracl.a" ]; then echo "Okay: Using $(PWD)/miracl.a ..."; echo; fi

	@if [ ! -f "miracl.h" ] && [ ! -f "MIRACL/miracl.h" ]; then echo "ERROR: cannot find miracl.h! I need a MIRACL :("; echo; exit 1; fi
	@if [ -f "miracl.h" ]; then echo "Okay: Using $(PWD)/miracl.h ..."; echo; fi
	@if [ ! -f "miracl.h" ]; then echo "Okay: Using $(PWD)/MIRACL/miracl.h ..."; cp MIRACL/miracl.h miracl.h; echo; fi

	@if [ ! -f "$(glib_inc)/glibconfig.h" ] && [ ! -f "$(glib_inc)/include/glibconfig.h" ] && [ ! -f "$(glib_inc)/glib/glibconfig.h" ] && [ ! -f "$(glib_dir)/include/glibconfig.h" ] && [ ! -f "$(glib_dir)/glib/glibconfig.h" ]; then echo "ERROR: glibconfig.h not found! Please install glib and modify 'glib_dir/glib_inc' ..."; echo; exit 1; fi

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
	$(CC) $(CCFLAGS) -shared DH1080.o miracl.a SHA-256.o base64.o -o DH1080.so
	@echo


misc:
	$(CC) $(CCFLAGS) -c -static SHA-256.c
	$(CC) $(CCFLAGS) -c -static base64.c
	$(CC) $(CCFLAGS) -c -static blowfish.c
	$(CC) $(CCFLAGS) -c -static cfgopts.c
	@echo


FiSH:
	$(CC) $(CCFLAGS) $(all_include_dirs) -static -Wall -c FiSH.c
	$(CC) $(CCFLAGS) -shared FiSH.o SHA-256.o base64.o blowfish.o cfgopts.o DH1080.o miracl.a -o libfish.so
	@echo


mac:	macosx
macosx:	note

	@if [ ! -f "mirdef_macosx.h" ] && [ ! -f "MIRACL/mirdef_macosx.h" ]; then echo "WARN: mirdef_macosx.h not found! :( Press ENTER to continue or CTRL+C to abort..."; read junk; echo; fi
	@if [ -f "mirdef_macosx.h" ]; then echo "Okay: Using $(PWD)/mirdef_macosx.h ..."; cp mirdef_macosx.h mirdef.h; echo; fi
	@if [ ! -f "mirdef_macosx.h" ]; then echo "Okay: Using $(PWD)/MIRACL/mirdef_macosx.h ..."; cp MIRACL/mirdef_macosx.h mirdef.h; echo; fi

	@# Eat this! gcc4/MacOSX (maybe you'll also need -fPIC)
	@# CCFLAGS="-fPIC -Wno-pointer-sign"
	@CCFLAGS='$(CCFLAGS) -fno-common'

	$(CC) $(CCFLAGS) -c SHA-256.c
	$(CC) $(CCFLAGS) -c base64.c
	$(CC) $(CCFLAGS) -c blowfish.c
	$(CC) $(CCFLAGS) -c cfgopts.c
	@echo

	ranlib miracl.a
	$(CC) $(CCFLAGS) -c DH1080.c
	$(CC) $(CCFLAGS) -bundle -flat_namespace -undefined suppress DH1080.o miracl.a SHA-256.o base64.o -o DH1080.so
	@echo

	$(CC) $(CCFLAGS) $(all_include_dirs) -Wall -c FiSH.c
	$(CC) $(CCFLAGS) -bundle -flat_namespace -undefined suppress FiSH.o SHA-256.o base64.o blowfish.o cfgopts.o DH1080.o miracl.a -o libfish.so

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

prefix=/opt/local
irssi_inc=${prefix}/include/irssi

CC=gcc
CFLAGS=-I${prefix}/include -I${irssi_inc} -I${irssi_inc}/src
CFLAGS+=-I${irssi_inc}/src/core -I${irssi_inc}/src/fe-common/core
CFLAGS+=`glib-config --cflags glib`
#LDFLAGS=-L${prefix}/lib -lgmp `glib-config --libs glib`
LDFLAGS=-L${prefix}/lib ${prefix}/lib/libgmp.a `glib-config --libs glib`
#LDFLAGS+=-bundle -flat_namespace -undefined suppress 
LDFLAGS+=-bundle  -undefined dynamic_lookup

TARGETS=SHA-256.o base64.o blowfish.o cfgopts.o DH1080.o FiSH.o

fish: ${TARGETS}
	${CC} $(CFLAGS) -o libfish.dylib ${TARGETS} ${LDFLAGS}

clean:
	rm -f ${TARGETS} libfish.dylib

/*
 *   MIRACL compiler/hardware definitions - mirdef.h
 *
 *   AMD64 processor (see miracl.zip/amd64.txt)
 *
 *   Copyright (c) 1988-2001 Shamus Software Ltd.
 */


#define MR_LITTLE_ENDIAN
#define MIRACL 64
#define mr_utype long		/* the underlying type is usually int *
							 * but see mrmuldv.any                */

#define MR_IBITS 32			/* bits in int  */
#define MR_LBITS 64			/* bits in long */
#define mr_unsign32 unsigned int
#define mr_unsign64 unsigned long
// #define MR_FLASH 52
#define MAXBASE ((mr_small)1<<(MIRACL-1))
#define BITSINCHAR 8

// #define MR_NOASM	// enable this if you have troubles with the inline-ASM parts

//#define MR_STRIPPED_DOWN


/* use assembly language file mrmuldv.s64
 *
 * Note that the above header file assumes an LP64-compatible compiler.
 * For an LLP64 compiler, change mr_utype to a 64-bit "long long" or __int64
 *
 * There is also a macro file amd64.mcs  - see kcmcomba.txt and makemcs.txt
 * However when we tried it the -O2 optimizer was broken when compiling mrcomba.c
 * or mrkcm.c
 */


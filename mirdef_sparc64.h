/*
 *   MIRACL compiler/hardware definitions - mirdef.h
 *
 *   sparc64 processor
 *
 *   Copyright (c) 1988-2001 Shamus Software Ltd.
 *
 *   NOTE: include compiler flag -m64. Also you may need to
 *   change -O2 to -O1
 */


#define MIRACL 32
#define MR_BIG_ENDIAN
#define mr_utype int
#define MR_IBITS 32
#define MR_LBITS 32
#define mr_dltype long
#define mr_unsign32 unsigned int
#define mr_unsign64 unsigned long
#define MAXBASE ((mr_small)1<<(MIRACL-1))

#define MR_NOASM	// enable this if you have troubles with the inline-ASM parts

//#define MR_STRIPPED_DOWN

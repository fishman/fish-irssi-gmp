/*
 *   MIRACL compiler/hardware definitions - mirdef.h
 *   Copyright (c) 1988-2002 Shamus Software Ltd.
 *
 *   Used with MIRACL v4.8.5 and MacOSX v10.3.7,
 *   Darwin Kernel v7.7.0
 */

#define MR_BIG_ENDIAN
#define MIRACL 32

#define mr_utype int		/* the underlying type is usually int *
							 * but see mrmuldv.any                */

#define MR_IBITS 32			/* bits in int  */
#define MR_LBITS 32			/* bits in long */
#define mr_unsign32 unsigned int
#define mr_dltype long long
#define mr_unsign64 unsigned long long
#define MR_FLASH 52
//#define MR_STRIPPED_DOWN
#define MAXBASE ((mr_small)1<<(MIRACL-1))
#define MR_BITSINCHAR 8

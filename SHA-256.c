#include <string.h>
#include "miracl.h"

// The actual SHA routines are included in MIRACL


// sha-256 a block of memory
void SHA256_memory(char *buf, int len, char *hash)
{
    sha256 md;

    shs256_init(&md);
	while(len--) shs256_process(&md, *buf++);
    shs256_hash(&md, hash);
    memset(&md, 0, sizeof(md));
}


// sha-256 a file, return 1 if ok
int sha_file(char *filename, char *hash)
{
	unsigned char buf[1024];
	unsigned int i, k;
	FILE *in;
	sha256 md;

	shs256_init(&md);
	in = fopen(filename, "rb");
	if(!in) return 0;

	do {
		i = fread(buf, 1, sizeof(buf), in);
		for(k=0; k<i; k++) shs256_process(&md, buf[k]);
	}
	while(i==sizeof(buf));

	shs256_hash(&md, hash);

	fclose(in);
	return 1;
}

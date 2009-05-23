#include <gmp.h>


// Input:  priv_key = buffer of 200 bytes
//         pub_key  = buffer of 200 bytes
// Output: priv_key = Your private key
//         pub_key  = Your public key
int DH1080_gen(char *priv_key, char *pub_key);

// Input:  MyPrivKey = Your private key
//         HisPubKey = Someones pubic key
// Output: MyPrivKey has been destroyed for security reasons
//         HisPubKey = the secret key
int DH1080_comp(char *MyPrivKey, char *HisPubKey);


#define ZeroMemory(dest,count) memset((void *)dest, 0, count)

void initb64();
int b64toh(char *b, char *d);
int htob64(char *h, char *d, unsigned int l);

extern unsigned char B64ABC[];	// original Base64 alphabet
extern unsigned char B64[];	// Base64 alphabet used by blowcrypt

void SHA256_memory(unsigned char *buf, int len, unsigned char *hash);
int sha_file(unsigned char *filename, unsigned char *hash);

void memXOR(unsigned char *s1, const unsigned char *s2, int n);

extern unsigned char iniPath[];

// return full path for ~/.irssi/config
extern const char *get_irssi_config(void);

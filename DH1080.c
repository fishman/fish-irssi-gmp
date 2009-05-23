// New Diffie-Hellman 1080bit Key-exchange

/* For Diffie-Hellman key-exchange a 1080bit germain prime is used, the
generator g=2 renders a field Fp from 1 to p-1. Therefore breaking it
means to solve a discrete logarithm problem with no less than 1080bit.

Base64 format is used to send the public keys over IRC.

The calculated secret key is hashed with SHA-256, the result is converted
to base64 for final use with blowfish. */



#include <stdio.h>
#include <time.h>
#include "DH1080.h"

// ### new sophie-germain 1080bit prime number ###
static char prime1080[135] = {
	0xFB, 0xE1, 0x02, 0x2E, 0x23, 0xD2, 0x13, 0xE8, 0xAC, 0xFA, 0x9A, 0xE8, 0xB9, 0xDF, 0xAD, 0xA3, 0xEA,
	0x6B, 0x7A, 0xC7, 0xA7, 0xB7, 0xE9, 0x5A, 0xB5, 0xEB, 0x2D, 0xF8, 0x58, 0x92, 0x1F, 0xEA, 0xDE, 0x95,
	0xE6, 0xAC, 0x7B, 0xE7, 0xDE, 0x6A, 0xDB, 0xAB, 0x8A, 0x78, 0x3E, 0x7A, 0xF7, 0xA7, 0xFA, 0x6A, 0x2B,
	0x7B, 0xEB, 0x1E, 0x72, 0xEA, 0xE2, 0xB7, 0x2F, 0x9F, 0xA2, 0xBF, 0xB2, 0xA2, 0xEF, 0xBE, 0xFA, 0xC8,
	0x68, 0xBA, 0xDB, 0x3E, 0x82, 0x8F, 0xA8, 0xBA, 0xDF, 0xAD, 0xA3, 0xE4, 0xCC, 0x1B, 0xE7, 0xE8, 0xAF,
	0xE8, 0x5E, 0x96, 0x98, 0xA7, 0x83, 0xEB, 0x68, 0xFA, 0x07, 0xA7, 0x7A, 0xB6, 0xAD, 0x7B, 0xEB, 0x61,
	0x8A, 0xCF, 0x9C, 0xA2, 0x89, 0x7E, 0xB2, 0x8A, 0x61, 0x89, 0xEF, 0xA0, 0x7A, 0xB9, 0x9A, 0x8A, 0x7F,
	0xA9, 0xAE, 0x29, 0x9E, 0xFA, 0x7B, 0xA6, 0x6D, 0xEA, 0xFE, 0xFB, 0xEF, 0xBF, 0x0B, 0x7D, 0x8B };

// base16: FBE1022E23D213E8ACFA9AE8B9DFADA3EA6B7AC7A7B7E95AB5EB2DF858921FEADE95E6AC7BE7DE6ADBAB8A783E7AF7A7FA6A2B7BEB1E72EAE2B72F9FA2BFB2A2EFBEFAC868BADB3E828FA8BADFADA3E4CC1BE7E8AFE85E9698A783EB68FA07A77AB6AD7BEB618ACF9CA2897EB28A6189EFA07AB99A8A7FA9AE299EFA7BA66DEAFEFBEFBF0B7D8B
// base10: 12745216229761186769575009943944198619149164746831579719941140425076456621824834322853258804883232842877311723249782818608677050956745409379781245497526069657222703636504651898833151008222772087491045206203033063108075098874712912417029101508315117935752962862335062591404043092163187352352197487303798807791605274487594646923


miracl *mip;
big b_prime1080, b_1;


BOOL DH1080_Init()
{
	initb64();
    mip=mirsys(256, 0);
	if(mip==NULL) return FALSE;

	b_prime1080=mirvar(0);
	b_1=mirvar(0);

	bytes_to_big(DH1080_PRIME_BYTES, prime1080, b_prime1080);

	lgconv(1, b_1);

	return TRUE;
}


void DH1080_DeInit()
{
	if(mip)
	{
		mirkill(b_1);
		mirkill(b_prime1080);
		mirexit();
	}
}



// verify the Diffie-Hellman public key as described in RFC 2631
BOOL DH_verifyPubKey(big b_pubkey)
{
	BOOL bRet = FALSE;

	// Verify that pubkey lies within the interval [2,p-1].
	// If it does not, the key is invalid.
	if(	(compare(b_pubkey, b_prime1080) == -1) &&
		(compare(b_pubkey, b_1) == 1)) bRet = TRUE;

	return bRet;
}



// Input:  priv_key = buffer of 200 bytes
//         pub_key  = buffer of 200 bytes
// Output: priv_key = Your private key
//         pub_key  = Your public key
int DH1080_gen(char *priv_key, char *pub_key)
{
	unsigned char raw_buf[160], iniHash[33];
	unsigned long seed;
	int len, iRet;

	big b_privkey, b_pubkey;
	csprng myRNG;

	FILE *hRnd;

	priv_key[0]='0';
	priv_key[1]='\0';
	pub_key[0]='0';
	pub_key[1]='\0';
	hRnd = fopen("/dev/urandom", "r");	// don't use /dev/random, it's a blocking device
	if(!hRnd) return 0;

	b_privkey=mirvar(0);
	b_pubkey=mirvar(0);

	// #*#*#*#*#* RNG START #*#*#*#*#*
	time((time_t *)&seed);

	seed ^= (long)hRnd << 16;
	if(fread(raw_buf, 1, sizeof(raw_buf), hRnd) < 32)
	{
		ZeroMemory(raw_buf, sizeof(raw_buf));
		fclose(hRnd);
		mirkill(b_privkey);
		mirkill(b_pubkey);

		return 0;
	}
	fclose(hRnd);

	sha_file(iniPath, iniHash);
	memXOR(raw_buf+128, iniHash, 32);
	sha_file((unsigned char *)get_irssi_config(), iniHash);
	memXOR(raw_buf+128, iniHash, 32);
	ZeroMemory(iniHash, sizeof(iniHash));
	// first 128 byte in raw_buf: output from /dev/urandom
	// last 32 byte in raw_buf: SHA-256 digest from blow.ini and irssi.conf

	seed *= (unsigned long)mip;
	strong_init(&myRNG, sizeof(raw_buf), raw_buf, (unsigned int)seed);
	strong_rng(&myRNG);
	strong_bigdig(&myRNG, 1080, 2, b_privkey);
	strong_kill(&myRNG);
	seed=0;
	// #*#*#*#*#* RNG END #*#*#*#*#*

	powltr(2, b_privkey, b_prime1080, b_pubkey);

	if(DH_verifyPubKey(b_pubkey))
	{
		len=big_to_bytes(sizeof(raw_buf), b_privkey, raw_buf, FALSE);
		htob64(raw_buf, priv_key, len);

		len=big_to_bytes(sizeof(raw_buf), b_pubkey, raw_buf, FALSE);
		htob64(raw_buf, pub_key, len);

		iRet=1;
	}
	else iRet=0;

	ZeroMemory(raw_buf, sizeof(raw_buf));

	mirkill(b_privkey);
	mirkill(b_pubkey);

	return iRet;
}



// Input:  MyPrivKey = Your private key
//         HisPubKey = Someones public key
// Output: MyPrivKey has been destroyed for security reasons
//         HisPubKey = the secret key
int DH1080_comp(char *MyPrivKey, char *HisPubKey)
{
	int i=0, len, iRet;
	unsigned char SHA256digest[35], base64_tmp[160];
	big b_myPrivkey, b_HisPubkey, b_theKey;

	// Verify base64 strings
	if((strspn(MyPrivKey, B64ABC) != strlen(MyPrivKey)) || (strspn(HisPubKey, B64ABC) != strlen(HisPubKey)))
	{
		memset(MyPrivKey, 0x20, strlen(MyPrivKey));
		memset(HisPubKey, 0x20, strlen(HisPubKey));
		return 0;
	}

	b_HisPubkey=mirvar(0);
	b_theKey=mirvar(0);


	len=b64toh(HisPubKey, base64_tmp);
	bytes_to_big(len, base64_tmp, b_HisPubkey);

	if(DH_verifyPubKey(b_HisPubkey))
	{
		b_myPrivkey=mirvar(0);

		len=b64toh(MyPrivKey, base64_tmp);
		bytes_to_big(len, base64_tmp, b_myPrivkey);
		memset(MyPrivKey, 0x20, strlen(MyPrivKey));

		powmod(b_HisPubkey, b_myPrivkey, b_prime1080, b_theKey);
		mirkill(b_myPrivkey);

		len=big_to_bytes(sizeof(base64_tmp), b_theKey, base64_tmp, FALSE);
		SHA256_memory(base64_tmp, len, SHA256digest);
		htob64(SHA256digest, HisPubKey, 32);

		iRet=1;
	}
	else iRet=0;


	ZeroMemory(base64_tmp, sizeof(base64_tmp));
	ZeroMemory(SHA256digest, sizeof(SHA256digest));

	mirkill(b_theKey);
	mirkill(b_HisPubkey);

	return iRet;
}

// New Diffie-Hellman 1080bit Key-exchange

/* For Diffie-Hellman key-exchange a 1080bit germain prime is used, the
generator g=2 renders a field Fp from 1 to p-1. Therefore breaking it
means to solve a discrete logarithm problem with no less than 1080bit.

Base64 format is used to send the public keys over IRC.

The calculated secret key is hashed with SHA-256, the result is converted
to base64 for final use with blowfish. */



#include <stdio.h>
#include <time.h>
#include <string.h>
#include "DH1080.h"

// ### new sophie-germain 1080bit prime number ###
//char *prime1080="++ECLiPSE+is+proud+to+present+latest+FiSH+release+featuring+even+more+security+for+you+++shouts+go+out+to+TMG+for+helping+to+generate+this+cool+sophie+germain+prime+number++++/C32L";
char *prime1080 = "FBE1022E23D213E8ACFA9AE8B9DFADA3EA6B7AC7A7B7E95AB5EB2DF858921FEADE95E6AC7BE7DE6ADBAB8A783E7AF7A7FA6A2B7BEB1E72EAE2B72F9FA2BFB2A2EFBEFAC868BADB3E828FA8BADFADA3E4CC1BE7E8AFE85E9698A783EB68FA07A77AB6AD7BEB618ACF9CA2897EB28A6189EFA07AB99A8A7FA9AE299EFA7BA66DEAFEFBEFBF0B7D8B";
// Base10: 12745216229761186769575009943944198619149164746831579719941140425076456621824834322853258804883232842877311723249782818608677050956745409379781245497526069657222703636504651898833151008222772087491045206203033063108075098874712912417029101508315117935752962862335062591404043092163187352352197487303798807791605274487594646923


// Input:  priv_key = buffer of 200 bytes
//         pub_key  = buffer of 200 bytes
// Output: priv_key = Your private key
//         pub_key  = Your public key
int DH1080_gen(char *priv_key, char *pub_key)
{
	unsigned char raw_buf[160];
	unsigned long seed;
	int len;

	mpz_t mpz_privkey, mpz_prime, mpz_pubkey, mpz_base;
	gmp_randstate_t randstate;

	priv_key[0]='0';
	priv_key[1]='\0';
	pub_key[0]='0';
	pub_key[1]='\0';

	initb64();

	mpz_init(mpz_privkey);
	mpz_init(mpz_pubkey);
	mpz_init_set_ui(mpz_base, 2);
	mpz_init_set_str(mpz_prime, prime1080, 16);

	// #*#*#*#*#* RNG START #*#*#*#*#*
	time((time_t *)&seed);

	gmp_randinit_default(randstate);
	gmp_randseed_ui(randstate, seed);
	mpz_urandomb(mpz_privkey, randstate, 1080);
	gmp_randclear(randstate);
	seed=0;
	// #*#*#*#*#* RNG END #*#*#*#*#*

	mpz_powm(mpz_pubkey, mpz_base, mpz_privkey, mpz_prime);

	mpz_export((void*)raw_buf, &len, 1, 1, 1, 0, mpz_privkey);
	mpz_clear(mpz_privkey);
	htob64(raw_buf, priv_key, len);

	mpz_export((void*)raw_buf, &len, 1, 1, 1, 0, mpz_pubkey);
	htob64(raw_buf, pub_key, len);
	ZeroMemory(raw_buf, sizeof(raw_buf));

	mpz_clear(mpz_pubkey);
	mpz_clear(mpz_prime);

	return 1;
}



// Input:  MyPrivKey = Your private key
//         HisPubKey = Someones public key
// Output: MyPrivKey has been destroyed for security reasons
//         HisPubKey = the secret key
int DH1080_comp(char *MyPrivKey, char *HisPubKey)
{
	int i=0, len;
	unsigned char SHA256digest[35], base64_tmp[160];
	mpz_t mpz_myPrivkey, mpz_hisPubkey, mpz_prime, mpz_theKey;

	// Verify base64 strings
	if((strspn(MyPrivKey, B64ABC) != strlen(MyPrivKey)) || (strspn(HisPubKey, B64ABC) != strlen(HisPubKey)))
	{
		memset(MyPrivKey, 0x20, strlen(MyPrivKey));
		memset(HisPubKey, 0x20, strlen(HisPubKey));
		return 0;
	}

	mpz_init(mpz_myPrivkey);
	mpz_init(mpz_hisPubkey);
	mpz_init(mpz_theKey);
	mpz_init_set_str(mpz_prime, prime1080, 16);

	len=b64toh(MyPrivKey, base64_tmp);
	mpz_import(mpz_myPrivkey, len, 1, 1, 1, 0, base64_tmp);
	memset(MyPrivKey, 0x20, strlen(MyPrivKey));

	len=b64toh(HisPubKey, base64_tmp);
	mpz_import(mpz_hisPubkey, len, 1, 1, 1, 0, base64_tmp);

	mpz_powm(mpz_theKey, mpz_hisPubkey, mpz_myPrivkey, mpz_prime);
	mpz_clear(mpz_myPrivkey);

	mpz_export((void*)base64_tmp, &len, 1, 1, 1, 0, mpz_theKey);
	mpz_clear(mpz_theKey);
	SHA256_memory(base64_tmp, len, SHA256digest);
	htob64(SHA256digest, HisPubKey, 32);
	ZeroMemory(base64_tmp, sizeof(base64_tmp));
	ZeroMemory(SHA256digest, sizeof(SHA256digest));

	mpz_clear(mpz_hisPubkey);
	mpz_clear(mpz_prime);
	return 1;
}

#include "cfgopts.h"
#include "DH1080.h"

#include <time.h>
#include <unistd.h>

// compile hack for irssi/src/common.h
#ifndef UOFF_T_INT
#define UOFF_T_INT
#endif

#include "module.h"


#define ZeroMemory(dest,count) memset((void *)dest, 0, count)

int decrypt_string(char *key, char *str, char *dest, int len);
int encrypt_string(char *key, char *str, char *dest, int len);

int encrypt_key(char *key, char *encryptedKey);

//int ExtractRhost (char *Rhost, char *incoming_msg);
int ExtractRnick (char *Rnick, char *incoming_msg);
void FixContactName(char *contactName);		// replace '[' and ']' in nick/channel with '~'

static void DH1080_received(SERVER_REC *server, char *msg, char *nick, char *address, char *target);

unsigned char default_iniKey[]="blowinikey\0ADDITIONAL SPACE FOR CUSTOM BLOW.INI PASSWORD";
unsigned char iniKey[100], blow_ini[]="/blow.ini", iniPath[255], tempPath[255];
char g_myPrivKey[300], g_myPubKey[300];

unsigned short int unsetiniFlag=0, keyx_query_created=0;

char *strfcpy(unsigned char *dest, char *buffer);	// removes leading and trailing blanks from string

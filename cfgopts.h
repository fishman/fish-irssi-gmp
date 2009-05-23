#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int WritePrivateProfileString(char *section, char *key, char *value, char *filepath);
int GetPrivateProfileString(char *section, char *key, char *default_value, char *buffer, int buflen, char *filepath);

extern unsigned char tempPath[];

// FiSH encryption module for irssi, v0.99

#include "FiSH.h"



// encrypt a message and store in bf_dest (using key for target)
static int FiSH_encrypt(char *msg_ptr, const char *target, char *bf_dest)
{
	unsigned char contactName[100]="", theKey[150]="", ini_outgoing[10]="";


	if(msg_ptr==NULL || *msg_ptr=='\0' || bf_dest==NULL || target==NULL || *target=='\0') return 0;

	GetPrivateProfileString("FiSH", "process_outgoing", "1", ini_outgoing, sizeof(ini_outgoing), iniPath);
	if(*ini_outgoing=='0' || *ini_outgoing=='n' || *ini_outgoing=='N') return 0;

	if(strlen(target) >= sizeof(contactName)) return 0;	// buffer too small
	strcpy(contactName, target);
	FixContactName(contactName);	// replace '[' and ']' with '~' in contact name

	GetPrivateProfileString(contactName, "key", "", theKey, sizeof(theKey), iniPath);
	if(strlen(theKey) < 4) return 0;		// don't process, key not found in ini

	if(strncmp(theKey, "+OK ", 4)==0)
	{	// key is encrypted, lets decrypt
		decrypt_string((char *)iniKey, theKey+4, theKey, strlen(theKey+4));
		if(*theKey=='\0')
		{	// don't process, decrypted key is bad
			ZeroMemory(theKey, sizeof(theKey));
			return 0;
		}
	}

	encrypt_string(theKey, msg_ptr, bf_dest, strlen(msg_ptr));
	ZeroMemory(theKey, sizeof(theKey));

	return 1;
}



// decrypt a base64 cipher text (using key for target)
static int FiSH_decrypt(char *msg_ptr, char *msg_bak, const char *target)
{
	unsigned char contactName[100], theKey[150]="", bf_dest[1000]="";
	unsigned char ini_incoming[10]="", myMark[20]="", mark_pos[20]="";
	unsigned int msg_len, i, mark_broken_block=0;


	if(msg_ptr==NULL || *msg_ptr=='\0' || msg_bak==NULL || target==NULL || *target=='\0') return 0;

	GetPrivateProfileString("FiSH", "process_incoming", "1", ini_incoming, sizeof(ini_incoming), iniPath);
	if(*ini_incoming=='0' || *ini_incoming=='n' || *ini_incoming=='N') return 0;

	if(strncmp(msg_ptr, "+OK ", 4)==0) msg_ptr += 4;
	else if(strncmp(msg_ptr, "mcps ", 5)==0) msg_ptr += 5;
	else return 0;		// don't process, blowcrypt-prefix not found

	strcpy(contactName, target);
	FixContactName(contactName);	// replace '[' and ']' with '~' in contact name

	GetPrivateProfileString(contactName, "key", "", theKey, sizeof(theKey), iniPath);
	if(*theKey=='\0' || strlen(theKey)<4) return 0;	// don't process, key not found in ini
	if(strncmp(theKey, "+OK ", 4)==0)
	{	// key is encrypted, lets decrypt
		decrypt_string((char *)iniKey, theKey+4, theKey, strlen(theKey+4));
		if(*theKey=='\0')
		{	// don't process, decrypted key is bad
			ZeroMemory(theKey, sizeof(theKey));
			return 0;
		}
	}

	// Verify base64 string
	msg_len=strlen(msg_ptr);
	if((strspn(msg_ptr, B64) != msg_len) || (msg_len < 12)) return 0;

	// usually a received message does not exceed 512 chars, but we want to prevent evil buffer overflow
	if(msg_len >= (int)(sizeof(bf_dest)*1.5)) msg_ptr[(int)(sizeof(bf_dest)*1.5)-20]='\0';

	// block-align blowcrypt strings if truncated by IRC server (each block is 12 chars long)
	// such a truncated block is destroyed and not needed anymore
	if(msg_len != (msg_len/12)*12)
	{
		msg_len=(msg_len/12)*12;
		msg_ptr[msg_len]='\0';
		GetPrivateProfileString("FiSH", "mark_broken_block", " \002&\002", myMark, sizeof(myMark), iniPath);
		if(*myMark=='\0' || *myMark=='0' || *myMark=='n' || *myMark=='N') mark_broken_block=0;
		else mark_broken_block=1;
	}

	decrypt_string(theKey, msg_ptr, bf_dest, msg_len);
	ZeroMemory(theKey, sizeof(theKey));

	i=0;
	while(bf_dest[i] != 0x0A && bf_dest[i] != 0x0D && bf_dest[i] != '\0') i++;
	bf_dest[i]='\0';	// in case of wrong key, decrypted message might have control characters -> cut message
	if(*bf_dest=='\0') return 0;	// don't process, decrypted msg is bad

	// append broken-block-mark?
	if(mark_broken_block) strcat(bf_dest, myMark);

	// append crypt-mark?
	GetPrivateProfileString(contactName, "mark_encrypted", "1", myMark, sizeof(myMark), iniPath);
	if(*myMark!='0' && *myMark!='n' && *myMark!='N')
	{
		GetPrivateProfileString("FiSH", "mark_encrypted", "", myMark, sizeof(myMark), iniPath);	// global setting
		if(*myMark != '\0')
		{
			GetPrivateProfileString("FiSH", "mark_position", "0", mark_pos, sizeof(mark_pos), iniPath);
			if(*mark_pos=='0') strcat(bf_dest, myMark);		//append mark at the end
			else
			{	// prefix mark
				i=strlen(myMark);
				memmove(bf_dest+i, bf_dest, strlen(bf_dest)+1);
				strncpy(bf_dest, myMark, i);
			}
		}
	}

	strcpy(msg_bak, bf_dest);	// copy decrypted message back (overwriting the base64 cipher text)
	ZeroMemory(bf_dest, sizeof(bf_dest));

	return 1;
}



static void decrypt_incoming(SERVER_REC *server, char *msg, const char *nick, const char *addr, const char *target)
{
	unsigned char contactName[100]="", *msg_bak;


	if(msg==NULL || target==NULL || nick==NULL) return;
	else if(strncmp(msg, "+OK ", 4)!=0 && strncmp(msg, "mcps ", 5)!=0 && strcmp(nick, "-psyBNC")!=0) return;	// don't process, blowcrypt-prefix not found

	msg_bak=(unsigned char *)msg;	// this is needed, because of psyBNC

	//channel?
	if(*target=='#' || *target=='&') strcpy(contactName, target);
	else if(strcmp(nick, "-psyBNC")==0)
	{
		msg=strstr(msg, " :(")+3;	// points to nick!ident@host in psybnc log
		if(msg==(char *)3) return;
		ExtractRnick(contactName, msg);
		msg=strchr(msg, ' ')+1;
		if(msg==(char *)1) return;
		if((strncmp(msg, "+OK ", 4)!=0) && (strncmp(msg, "mcps ", 5)!=0)) return;
	}
	else strcpy(contactName, nick);

	if(FiSH_decrypt(msg, msg, contactName))
	{
		signal_stop();
		signal_emit(signal_get_emitted(), 5, server, msg_bak, nick, addr, target);
	}
}



static void encrypt_outgoing(SERVER_REC *server, char *msg, char *target, char *orig_target)
{
	unsigned char bf_dest[800]="";
	unsigned char contactName[100]="", plain_prefix[20]="";
	unsigned char myMark[20]="", mark_pos[20]="";
	unsigned int i;


	// generally cut a message to a size of 512 byte, as everything above will never arrive complete anyway
	if(strlen(msg) > 512) msg[512]='\0';

	// plain-prefix in msg found?
	GetPrivateProfileString("FiSH", "plain_prefix", "+p ", plain_prefix, sizeof(plain_prefix), iniPath);
	if(*plain_prefix != '\0')
	{
		i=strlen(plain_prefix);
		if(strncasecmp(msg, plain_prefix, i)==0)
		{
			irc_send_cmdv(server, "PRIVMSG %s :%s", target, msg+i);
			return;
		}
	}

	if(FiSH_encrypt(msg, target, bf_dest)==0)
	{	// send plain-text (no key found)
		irc_send_cmdv(server, "PRIVMSG %s :%s", target, msg);
		return;
	}

	bf_dest[512]='\0';
	irc_send_cmdv(server, "PRIVMSG %s :%s %s\n", target, "+OK", bf_dest);

	strcpy(bf_dest, msg);

	// append crypt-mark?
	GetPrivateProfileString(contactName, "mark_encrypted", "1", myMark, sizeof(myMark), iniPath);
	if(*myMark!='0' && *myMark!='n' && *myMark!='N')
	{
		GetPrivateProfileString("FiSH", "mark_encrypted", "", myMark, sizeof(myMark), iniPath);	// global setting
		if(*myMark != '\0')
		{
			GetPrivateProfileString("FiSH", "mark_position", "0", mark_pos, sizeof(mark_pos), iniPath);
			if(*mark_pos=='0') strcat(bf_dest, myMark);		//append mark at the end
			else
			{	// prefix mark
				i=strlen(myMark);
				memmove(bf_dest+i, bf_dest, strlen(bf_dest)+1);
				strncpy(bf_dest, myMark, i);
			}
		}
	}

	signal_continue(4, server, bf_dest, target, orig_target);
	ZeroMemory(bf_dest, sizeof(bf_dest));

	return;
}



static void notice_incoming(SERVER_REC *server, char *msg, char *nick, char *address, char *target)
{	// decrypt NOTICE messages (and forward DH1080 key-exchange)
	unsigned char *contactPtr;

	if(strncmp(msg, "DH1024_", 7)==0)
	{
		irc_send_cmdv(server, "NOTICE %s :Received \002old DH1024\002 public key from you! Please update to latest version: http://fish.sekure.us\n", nick);
		printtext(server, NULL, MSGLEVEL_CRAP, "\002FiSH:\002 Received \002old DH1024\002 public key from %s! Telling him to update...", nick);
		return;
	}

	if(strncmp(msg, "DH1080_", 7)==0)
	{
		DH1080_received(server, msg, nick, address, target);
		return;
	}

	if(*target=='#' || *target=='&') contactPtr=target;
	else contactPtr=nick;

	if(FiSH_decrypt(msg, msg, contactPtr))
	{
		signal_stop();
		signal_emit(signal_get_emitted(), 5, server, msg, nick, address, target);
	}
}



static void decrypt_action(SERVER_REC *server, char *msg, char *nick, char *address, char *target)
{
	if(FiSH_decrypt(msg, msg, target))
	{
		signal_stop();
		signal_emit(signal_get_emitted(), 5, server, msg, nick, address, target);
	}
}



static void decrypt_topic(SERVER_REC *server, char *channel, char *topic, char *nick, char *address)
{
	if(FiSH_decrypt(topic, topic, channel))
	{
		signal_stop();
		signal_emit(signal_get_emitted(), 5, server, channel, topic, nick, address);
	}
}



static void topic_changed(CHANNEL_REC *chan_rec)
{
	FiSH_decrypt(chan_rec->topic, chan_rec->topic, chan_rec->name);
}



static void raw_handler(SERVER_REC *server, char *data)
{
	char channel[100], *ptr, *ptr_bak;
	int len;

	if(data==NULL || *data=='\0') return;

	ptr=strchr(data, ' ');	// point to command
	if(ptr==NULL) return;
	ptr++;

	if(strncmp(ptr, "332 ", 4)!=0) return;	// 332 = TOPIC

	ptr_bak=ptr;
	ptr=strchr(ptr, '#');	// point to #channel
	if(ptr==NULL)
	{
		ptr=strchr(ptr_bak, '&');	// point to &channel
		if(ptr_bak==NULL) return;
	}

	len=strchr(ptr, ' ')-ptr;
	if(len >= sizeof(channel)-2) return;	// channel string too long, something went wrong
	strncpy(channel, ptr, len);
	channel[len]='\0';

	ptr=strchr(ptr, ':');	// point to topic msg start
	if(ptr==NULL) return;
	ptr++;

	FiSH_decrypt(ptr, ptr, channel);
}



static void encrypt_notice(const char *data, SERVER_REC *server, WI_ITEM_REC *item)
{	// New command: /notice+ <nick/#channel> <notice message>
	unsigned char bf_dest[1000]="";
	const char *target;
	unsigned char *msg;
	void *free_arg=NULL;


	if(data==NULL || (strlen(data) < 3)) goto notice_error;
	if(!cmd_get_params(data, &free_arg, 2, &target, &msg)) goto notice_error;

	if (target==NULL || *target=='\0' || msg==NULL || *msg=='\0') goto notice_error;

	// generally refuse a notice size of more than 512 byte, as everything above will never arrive complete anyway
	if(strlen(msg) >= 512)
	{
		printtext(server, target, MSGLEVEL_CRAP, "\002FiSH:\002 /notice+ error: message argument exceeds buffer size!");
		goto notice_error;
	}

	if(FiSH_encrypt(msg, target, bf_dest)==0)
	{
		printtext(server, target, MSGLEVEL_CRAP, "\002FiSH:\002 /notice+ error: Encryption disabled or no key found for %s.", target);
		goto notice_error;
	}

	bf_dest[512]='\0';
	irc_send_cmdv(server, "NOTICE %s :%s %s\n", target, "+OK", bf_dest);

	signal_emit("message irc own_notice", 3, server, msg, target);
	cmd_params_free(free_arg);
	return;

notice_error:
	if(free_arg) cmd_params_free(free_arg);
	printtext(server, item!=NULL ? window_item_get_target(item) : NULL, MSGLEVEL_CRAP,
			"\002FiSH:\002 Usage: /notice+ <nick/#channel> <notice message>");
}



// set encrypted topic for current channel, irssi syntax: /topic+ <your topic>
static void command_crypt_TOPIC(const char *data, SERVER_REC *server, WI_ITEM_REC *item)
{
	unsigned char bf_dest[1000]="";
	const char *target;


	if(data==0 || *data=='\0') goto topic_error;
	if(item!=NULL) target=window_item_get_target(item);
	else goto topic_error;

	if(*target!='#' && *target!='&')
	{
		printtext(server, target, MSGLEVEL_CRAP, "\002FiSH:\002 Please change to the channel window where you want to set the topic!");
		goto topic_error;
	}

	// generally refuse a topic size of more than 512 byte, as everything above will never arrive complete anyway
	if(strlen(data) >= 512)
	{
		printtext(server, target, MSGLEVEL_CRAP, "\002FiSH:\002 /topic+ error: topic length exceeds buffer size!");
		goto topic_error;
	}

	// encrypt a message (using key for target)
	if(FiSH_encrypt((char *)data, target, bf_dest)==0)
	{
		printtext(server, target, MSGLEVEL_CRAP, "\002FiSH:\002 /topic+ error: Encryption disabled or no key found for %s.", target);
		goto topic_error;
	}

	bf_dest[512]='\0';
	irc_send_cmdv(server, "TOPIC %s :%s %s\n", target, "+OK", bf_dest);
	return;

topic_error:
	printtext(server, item!=NULL ? window_item_get_target(item) : NULL, MSGLEVEL_CRAP,
			"\002FiSH:\002 Usage: /topic+ <your new topic>");
}



static void command_helpfish(const char *arg, SERVER_REC *server, WI_ITEM_REC *item)
{
	printtext(NULL, NULL, MSGLEVEL_CRAP,
		"\n\002FiSH HELP:\002 For more information read FiSH-irssi.txt :)\n\n"
		" /topic+ <your new topic>\n"
		" /notice+ <nick/#channel> <notice message>\n"
		" /key [<nick/#channel>]\n"
		" /setkey [<nick/#channel>] <sekure_key>\n"
		" /delkey <nick/#channel>\n"
		" /keyx [<nick>] (DH1080 KeyXchange)\n"
		" /setinipw <sekure_blow.ini_password>\n"
		" /unsetinipw\n");
}



static void command_setinipw(const char *iniPW, SERVER_REC *server, WI_ITEM_REC *item)
{
	unsigned int i=0, pw_len, re_enc=0;
	unsigned char B64digest[50], SHA256digest[35];
	unsigned char bfKey[512], new_iniKey[100], old_iniKey[100], *fptr, *ok_ptr, line_buf[1000], iniPath_new[300];
	FILE *h_ini, *h_ini_new;

	pw_len=strlen(iniPW);
	if(pw_len < 1 || pw_len > sizeof(new_iniKey))
	{
		printtext(server, item!=NULL ? window_item_get_target(item) : NULL, MSGLEVEL_CRAP,
				"\002FiSH:\002 No parameters. Usage: /setinipw <sekure_blow.ini_password>");
		return;
	}

	strfcpy(new_iniKey, (char *)iniPW);
	ZeroMemory(iniPW, pw_len);
	pw_len=strlen(new_iniKey);

	if(pw_len < 8)
	{
		printtext(server, item!=NULL ? window_item_get_target(item) : NULL, MSGLEVEL_CRAP,
				"\002FiSH:\002 Password too short, at least 8 characters needed! Usage: /setinipw <sekure_blow.ini_password>");
		return;
	}

	SHA256_memory(new_iniKey, pw_len, SHA256digest);
	ZeroMemory(new_iniKey, sizeof(new_iniKey));
	for(i=0;i<40872;i++) SHA256_memory(SHA256digest, 32, SHA256digest);
	htob64(SHA256digest, B64digest, 32);
	strcpy(old_iniKey, iniKey);
	if(unsetiniFlag) strcpy(iniKey, default_iniKey);	// unsetinipw
	else strcpy(iniKey, B64digest);	// this is used for encrypting blow.ini
	for(i=0;i<30752;i++) SHA256_memory(SHA256digest, 32, SHA256digest);
	htob64(SHA256digest, B64digest, 32);	// this is used to verify the entered password
	ZeroMemory(SHA256digest, sizeof(SHA256digest));


	// re-encrypt blow.ini with new password
	strcpy(iniPath_new, iniPath);
	strcat(iniPath_new, "_new");
	h_ini_new=fopen(iniPath_new, "w");
	h_ini=fopen(iniPath,"r");
	if(h_ini && h_ini_new)
	{
		while (!feof(h_ini))
		{
			fptr=fgets(line_buf, sizeof(line_buf)-2, h_ini);
			if(fptr)
			{
				ok_ptr=strstr(line_buf, "+OK ");
				if(ok_ptr)
				{
					re_enc=1;
					strtok(ok_ptr+4, " \n\r");
					decrypt_string(old_iniKey, ok_ptr+4, bfKey, strlen(ok_ptr+4));
					ZeroMemory(ok_ptr+4, strlen(ok_ptr+4)+1);
					encrypt_string(iniKey, bfKey, ok_ptr+4, strlen(bfKey));
					strcat(line_buf, "\n");
				}
				if(fprintf(h_ini_new, "%s", line_buf) < 0)
				{
					ZeroMemory(B64digest, sizeof(B64digest));
					ZeroMemory(bfKey, sizeof(bfKey));
					ZeroMemory(line_buf, sizeof(line_buf));
					ZeroMemory(old_iniKey, sizeof(old_iniKey));
					fclose(h_ini);
					fclose(h_ini_new);
					remove(iniPath_new);

					printtext(server, item!=NULL ? window_item_get_target(item) : NULL,	MSGLEVEL_CRAP,
							"\002FiSH ERROR:\002 Unable to write new blow.ini, probably out of disc space.");

					return;
				}
			}
		}

		ZeroMemory(bfKey, sizeof(bfKey));
		ZeroMemory(line_buf, sizeof(line_buf));
		ZeroMemory(old_iniKey, sizeof(old_iniKey));
		fclose(h_ini);
		fclose(h_ini_new);
		remove(iniPath);
		rename(iniPath_new, iniPath);
	}
	else return;

	if(WritePrivateProfileString("FiSH", "ini_password_Hash", B64digest, iniPath) == -1)
	{
		ZeroMemory(B64digest, sizeof(B64digest));
		printtext(server, item!=NULL ? window_item_get_target(item) : NULL,	MSGLEVEL_CRAP,
				"\002FiSH ERROR:\002 Unable to write to blow.ini, probably out of space or permission denied.");
		return;
	}

	ZeroMemory(B64digest, sizeof(B64digest));

	if(re_enc) printtext(server, item!=NULL ? window_item_get_target(item) : NULL,
					MSGLEVEL_CRAP, "\002FiSH: Re-encrypted blow.ini\002 with new password.");

	if(!unsetiniFlag) printtext(server, item!=NULL ? window_item_get_target(item) : NULL,
						MSGLEVEL_CRAP, "\002FiSH:\002 blow.ini password hash saved.");
}



// Change back to default blow.ini password, irssi syntax: /unsetinipw
static void command_unsetinipw(const char *arg, SERVER_REC *server, WI_ITEM_REC *item)
{
	unsetiniFlag=1;
	command_setinipw("Some_boogie_dummy_key", server, item);
	unsetiniFlag=0;
	if(WritePrivateProfileString("FiSH", "ini_password_Hash", "\0", iniPath) == -1)
	{
		printtext(server, item!=NULL ? window_item_get_target(item) : NULL,	MSGLEVEL_CRAP,
			"\002FiSH ERROR:\002 Unable to write to blow.ini, probably out of space or permission denied.");
		return;
	}
	printtext(server, item!=NULL ? window_item_get_target(item) : NULL, MSGLEVEL_CRAP,
		"\002FiSH:\002 Changed back to default blow.ini password, you won't have to enter it on start-up anymore!");
}



static void command_setkey(const char *data, SERVER_REC *server, WI_ITEM_REC *item)
{
	unsigned char contactName[100]="", encryptedKey[150]="";
	const char *target, *key;
	void *free_arg;

	if (data==NULL || *data=='\0')
	{
		printtext(server, item!=NULL ? window_item_get_target(item) : NULL, MSGLEVEL_CRAP,
			"\002FiSH:\002 No parameters. Usage: /setkey [<nick/#channel>] <sekure_key>");
		return;
	}

	if (!cmd_get_params(data, &free_arg, 2, &target, &key)) return;

	if (*target=='\0')
	{
		printtext(server, item!=NULL ? window_item_get_target(item) : NULL, MSGLEVEL_CRAP,
				"\002FiSH:\002 No parameters. Usage: /setkey [<nick/#channel>] <sekure_key>");
		cmd_params_free(free_arg);
		return;
	}

	if (*key=='\0') {
		// one paramter given - it's the key
		key = target;
		if (item != NULL) target = window_item_get_target(item);
		else
		{
			printtext(NULL, NULL, MSGLEVEL_CRAP,
					"\002FiSH:\002 Please define nick/#channel. Usage: /setkey [<nick/#channel>] <sekure_key>");
			cmd_params_free(free_arg);
			return;
		}
	}

	strcpy(contactName, target);
	FixContactName(contactName);

	encrypt_key((char *)key, encryptedKey);

	if(WritePrivateProfileString(contactName, "key", encryptedKey, iniPath) == -1)
	{
		ZeroMemory(encryptedKey, sizeof(encryptedKey));
		printtext(server, item!=NULL ? window_item_get_target(item) : NULL,	MSGLEVEL_CRAP,
				"\002FiSH ERROR:\002 Unable to write to blow.ini, probably out of space or permission denied.");
		cmd_params_free(free_arg);
		return;
	}

	ZeroMemory(encryptedKey, sizeof(encryptedKey));

	printtext(server, item!=NULL ? window_item_get_target(item) : NULL, MSGLEVEL_CRAP,
			"\002FiSH:\002 Key for %s successfully set!", target);

	cmd_params_free(free_arg);
}



static void command_delkey(const char *target, SERVER_REC *server, WI_ITEM_REC *item)
{
	unsigned char contactName[100]="";

	if (target==NULL || *target=='\0')
	{
		printtext(server, item!=NULL ? window_item_get_target(item) : NULL, MSGLEVEL_CRAP,
				"\002FiSH:\002 No parameters. Usage: /delkey <nick/#channel>");
		return;
	}

	strfcpy(contactName, (char *)target);
	FixContactName(contactName);

	if(WritePrivateProfileString(contactName, "key", "\0", iniPath) == -1)
	{
		printtext(server, item!=NULL ? window_item_get_target(item) : NULL,	MSGLEVEL_CRAP,
			"\002FiSH ERROR:\002 Unable to write to blow.ini, probably out of space or permission denied.");
		return;
	}

	printtext(server, item!=NULL ? window_item_get_target(item) : NULL,	MSGLEVEL_CRAP,
		"\002FiSH:\002 Key for %s successfully removed!", target);
}



static void command_key(const char *target, SERVER_REC *server, WI_ITEM_REC *item)
{
	unsigned char contactName[100]="", theKey[150]="";

	if(target==NULL || *target=='\0')
	{
		if (item!=NULL) target=window_item_get_target(item);
		else
		{
			printtext(NULL, NULL, MSGLEVEL_CRAP, "\002FiSH:\002 Please define nick/#channel. Usage: /key <nick/#channel>");
			return;
		}
	}

	strfcpy(contactName, (char *)target);
	FixContactName(contactName);

	GetPrivateProfileString(contactName, "key", "", theKey, sizeof(theKey), iniPath);
	if(*theKey=='\0' || strlen(theKey)<4)
	{	// don't process, key not found in ini
		printtext(server, item!=NULL ? window_item_get_target(item) : NULL, MSGLEVEL_CRAP,
			"\002FiSH:\002 Key for %s not found!", target);
		return;
	}

	if(strncmp(theKey, "+OK ", 4)==0)
	{       // key is encrypted, lets decrypt
		decrypt_string((char *)iniKey, theKey+4, theKey, strlen(theKey+4));
		if(*theKey=='\0')
		{	// don't process, decrypted key is bad
			ZeroMemory(theKey, sizeof(theKey));
			printtext(server, target, MSGLEVEL_CRAP, "\002FiSH:\002 Key for %s invalid!", target);
			return;
		}
	}

	printtext(server, target, MSGLEVEL_CRAP, "\002FiSH:\002 Key for %s: %s", target, theKey);

	ZeroMemory(theKey, sizeof(theKey));
}



static void command_keyx(const char *target, SERVER_REC *server, WI_ITEM_REC *item)
{
	if(target==NULL || *target=='\0')
	{
		if(item!=NULL) target = window_item_get_target(item);
		else
		{
			printtext(NULL, NULL, MSGLEVEL_CRAP, "\002FiSH:\002 Please define nick/#channel. Usage: /keyx <nick/#channel>");
			return;
		}
	}

	if(*target=='#' || *target=='&')
	{
		printtext(server, target, MSGLEVEL_CRAP, "\002FiSH:\002 KeyXchange does not work for channels!");
		return;
	}

	DH1080_gen(g_myPrivKey, g_myPubKey);

	irc_send_cmdv(server, "NOTICE %s :%s %s", target, "DH1080_INIT", g_myPubKey);

	printtext(server, item!=NULL ? window_item_get_target(item) : NULL, MSGLEVEL_CRAP,
			"\002FiSH:\002 Sent my DH1080 public key to %s, waiting for reply ...", target);
}



static void DH1080_received(SERVER_REC *server, char *msg, char *nick, char *address, char *target)
{
	unsigned int i;
	char hisPubKey[300], contactName[100]="", encryptedKey[150]="";

	if(*target=='&' || *target=='#' || *nick=='#' || *nick=='&') return;	// no KeyXchange for channels...
	i=strlen(msg);
	if(i<191 || i>195) return;

	if(strncmp(msg, "DH1080_INIT ", 12)==0)
	{
		strcpy(hisPubKey, msg+12);
		if(strspn(hisPubKey, B64ABC) != strlen(hisPubKey)) return;

		if(query_find(server, nick)==NULL)
		{	// query window not found, lets create one
			keyx_query_created=1;
			irc_query_create(server->tag, nick, TRUE);
			keyx_query_created=0;
		}

		printtext(server, nick, MSGLEVEL_CRAP, "\002FiSH:\002 Received DH1080 public key from %s, sending mine...", nick);

		DH1080_gen(g_myPrivKey, g_myPubKey);
		irc_send_cmdv(server, "NOTICE %s :%s %s", nick, "DH1080_FINISH", g_myPubKey);
	}
	else if(strncmp(msg, "DH1080_FINISH ", 14)==0) strcpy(hisPubKey, msg+14);
	else return;

	if(DH1080_comp(g_myPrivKey, hisPubKey)==0) return;
	signal_stop();

	strcpy(contactName, nick);
	FixContactName(contactName);

	encrypt_key(hisPubKey, encryptedKey);
	ZeroMemory(hisPubKey, sizeof(hisPubKey));

	if(WritePrivateProfileString(contactName, "key", encryptedKey, iniPath) == -1)
	{
		ZeroMemory(encryptedKey, sizeof(encryptedKey));
		printtext(server, nick,	MSGLEVEL_CRAP, "\002FiSH ERROR:\002 Unable to write to blow.ini, probably out of space or permission denied.");
		return;
	}

	ZeroMemory(encryptedKey, sizeof(encryptedKey));

	printtext(server, nick, MSGLEVEL_CRAP, "\002FiSH:\002 Key for %s successfully set!", nick);
}



static void do_auto_keyx(QUERY_REC *query, int automatic)
{	// perform auto-keyXchange only for known people
	unsigned char theKey[150]="", ini_auto_keyx[10], contactName[100]="";

	if(keyx_query_created) return;	// query was created by FiSH

	GetPrivateProfileString("FiSH", "auto_keyxchange", "1", ini_auto_keyx, sizeof(ini_auto_keyx), iniPath);
	if(*ini_auto_keyx=='0' || *ini_auto_keyx=='N' || *ini_auto_keyx=='n') return;

	strcpy(contactName, query->name);
	FixContactName(contactName);

	GetPrivateProfileString(contactName, "key", "", theKey, sizeof(theKey), iniPath);
	if(strlen(theKey) > 4)
	{
		ZeroMemory(theKey, sizeof(theKey));
		command_keyx(query->name, query->server, NULL);
	}
}



static void query_nick_changed(QUERY_REC *query, char *orignick)
{	// copy key for old nick to use with the new one
	unsigned char theKey[150]="", ini_nicktracker[10], contactName[100]="";

	GetPrivateProfileString("FiSH", "nicktracker", "1", ini_nicktracker, sizeof(ini_nicktracker), iniPath);
	if(*ini_nicktracker=='0' || *ini_nicktracker=='N' || *ini_nicktracker=='n') return;

	if(orignick==NULL || strcasecmp(orignick, query->name)==0) return;	// same nick, different case?
	strcpy(contactName, orignick);
	FixContactName(contactName);

	GetPrivateProfileString(contactName, "key", "", theKey, sizeof(theKey), iniPath);
	if(strlen(theKey) < 4) return;	// see if there is a key for the old nick

	strcpy(contactName, query->name);
	FixContactName(contactName);

	if(WritePrivateProfileString(contactName, "key", theKey, iniPath) == -1)
	{
		ZeroMemory(theKey, sizeof(theKey));
		printtext(NULL, NULL, MSGLEVEL_CRAP, "\002FiSH ERROR:\002 Unable to write to blow.ini, probably out of space or permission denied.");
		return;
	}

	ZeroMemory(theKey, sizeof(theKey));
}



static void void_send(SERVER_REC *server, const char *target, const char *msg, int target_type)
{
}

static void server_register_fish(SERVER_REC *server)
{
	MODULE_SERVER_REC *rec;

	if(server==NULL) return;

	rec = g_new0(MODULE_SERVER_REC, 1);
	MODULE_DATA_SET(server, rec);
	rec->orig_send_message = server->send_message;
	server->send_message = void_send;
}

static void server_unregister_fish(SERVER_REC *server)
{
	MODULE_SERVER_REC *rec;

	if(server==NULL) return;

	rec = MODULE_DATA(server);
	if(rec==NULL) return;
	server->send_message = rec->orig_send_message;
	MODULE_DATA_UNSET(server);
	g_free(rec);
}



void fish_init(void)
{
	unsigned char iniPasswordHash[50], SHA256digest[35], B64digest[50], *iniPass_ptr;
	unsigned int i;

	g_slist_foreach(servers, (GFunc) server_register_fish, NULL);

	strcpy(iniPath, get_irssi_config());	// path to irssi config file
	strcpy(tempPath, iniPath);
	strcpy(strrchr(iniPath, '/'), blow_ini);
	strcpy(strrchr(tempPath, '/'), "/temp_FiSH.$$$");

	GetPrivateProfileString("FiSH", "ini_Password_hash", "0", iniPasswordHash, sizeof(iniPasswordHash), iniPath);
	if(strlen(iniPasswordHash) == 43)
	{
		iniPass_ptr = getpass(" --> Please enter your blow.ini password: ");
		strcpy(iniKey, iniPass_ptr);
		ZeroMemory(iniPass_ptr, strlen(iniPass_ptr));
		irssi_redraw();		// getpass() screws irssi GUI, lets redraw!

		SHA256_memory(iniKey, strlen(iniKey), SHA256digest);
		for(i=0;i<40872;i++) SHA256_memory(SHA256digest, 32, SHA256digest);
		htob64(SHA256digest, B64digest, 32);
		strcpy(iniKey, B64digest);      // this is used for encrypting blow.ini
		for(i=0;i<30752;i++) SHA256_memory(SHA256digest, 32, SHA256digest);
		htob64(SHA256digest, B64digest, 32);	// this is used to verify the entered password
		if(strcmp(B64digest, iniPasswordHash) != 0)
		{
			printtext(NULL, NULL, MSGLEVEL_CRAP, "\002FiSH:\002 Wrong blow.ini password entered, try again...");
			printtext(NULL, NULL, MSGLEVEL_CRAP, "\002FiSH module NOT loaded.\002");
			return;
		}
		printtext(NULL, NULL, MSGLEVEL_CRAP, "\002FiSH:\002 Correct blow.ini password entered, lets go!");
	}
	else
	{
		strcpy(iniKey, default_iniKey);
		printtext(NULL, NULL, MSGLEVEL_CRAP, "\002FiSH:\002 Using default password to decrypt blow.ini... Try /setinipw to set a custom password.");
	}


	signal_add_first("message private", (SIGNAL_FUNC) decrypt_incoming);
	signal_add_first("message public", (SIGNAL_FUNC) decrypt_incoming);
	signal_add_first("message irc notice", (SIGNAL_FUNC) notice_incoming);
	signal_add_first("message irc action", (SIGNAL_FUNC) decrypt_action);

	signal_add_first("message own_private", (SIGNAL_FUNC) encrypt_outgoing);
	signal_add_first("message own_public", (SIGNAL_FUNC) encrypt_outgoing);

	signal_add_first("channel topic changed", (SIGNAL_FUNC) topic_changed);
	signal_add_first("message topic", (SIGNAL_FUNC) decrypt_topic);
	signal_add_first("server incoming", (SIGNAL_FUNC) raw_handler);

	signal_add("server connected", (SIGNAL_FUNC) server_register_fish);
	signal_add("server disconnected", (SIGNAL_FUNC) server_unregister_fish);

	signal_add("query created", (SIGNAL_FUNC) do_auto_keyx);
	signal_add("query nick changed", (SIGNAL_FUNC) query_nick_changed);

	command_bind("topic+", NULL, (SIGNAL_FUNC) command_crypt_TOPIC);
	command_bind("notice+", NULL, (SIGNAL_FUNC) encrypt_notice);
	command_bind("notfish", NULL, (SIGNAL_FUNC) encrypt_notice);
	command_bind("setkey", NULL, (SIGNAL_FUNC) command_setkey);
	command_bind("delkey", NULL, (SIGNAL_FUNC) command_delkey);
	command_bind("key", NULL, (SIGNAL_FUNC) command_key);
	command_bind("keyx", NULL, (SIGNAL_FUNC) command_keyx);
	command_bind("setinipw", NULL, (SIGNAL_FUNC) command_setinipw);
	command_bind("unsetinipw", NULL, (SIGNAL_FUNC) command_unsetinipw);

	command_bind("fishhelp", NULL, (SIGNAL_FUNC) command_helpfish);
	command_bind("helpfish", NULL, (SIGNAL_FUNC) command_helpfish);

	printtext(NULL, NULL, MSGLEVEL_CLIENTNOTICE,
		"FiSH v0.99 - encryption module for irssi loaded! URL: http://fish.sekure.us\n"
		"Try /helpfish or /fishhelp for a short command overview");

	module_register("fish", "core");
}



void fish_deinit(void)
{
	g_slist_foreach(servers, (GFunc) server_unregister_fish, NULL);

	signal_remove("message private", (SIGNAL_FUNC) decrypt_incoming);
	signal_remove("message public", (SIGNAL_FUNC) decrypt_incoming);
	signal_remove("message irc notice", (SIGNAL_FUNC) notice_incoming);
	signal_remove("message irc action", (SIGNAL_FUNC) decrypt_action);

	signal_remove("message own_private", (SIGNAL_FUNC) encrypt_outgoing);
	signal_remove("message own_public", (SIGNAL_FUNC) encrypt_outgoing);

	signal_remove("channel topic changed", (SIGNAL_FUNC) topic_changed);
	signal_remove("message topic", (SIGNAL_FUNC) decrypt_topic);
	signal_remove("server incoming", (SIGNAL_FUNC) raw_handler);

	signal_remove("server connected", (SIGNAL_FUNC) server_register_fish);
	signal_remove("server disconnected", (SIGNAL_FUNC) server_unregister_fish);

	signal_remove("query created", (SIGNAL_FUNC) do_auto_keyx);
	signal_remove("query nick changed", (SIGNAL_FUNC) query_nick_changed);

	command_unbind("topic+", (SIGNAL_FUNC) command_crypt_TOPIC);
	command_unbind("notice+", (SIGNAL_FUNC) encrypt_notice);
	command_unbind("notfish", (SIGNAL_FUNC) encrypt_notice);
	command_unbind("setkey", (SIGNAL_FUNC) command_setkey);
	command_unbind("delkey", (SIGNAL_FUNC) command_delkey);
	command_unbind("key", (SIGNAL_FUNC) command_key);
	command_unbind("keyx", (SIGNAL_FUNC) command_keyx);
	command_unbind("setinipw", (SIGNAL_FUNC) command_setinipw);
	command_unbind("unsetinipw", (SIGNAL_FUNC) command_unsetinipw);

	command_unbind("fishhelp", (SIGNAL_FUNC) command_helpfish);
	command_unbind("helpfish", (SIGNAL_FUNC) command_helpfish);
}



// :someone!ident@host.net PRIVMSG leetguy :Some Text -> Result: Rnick="someone"
int ExtractRnick(char *Rnick, char *incoming_msg)		// needs direct pointer to "nick@host" or ":nick@host"
{
	int k=0;

	if(*incoming_msg == ':') incoming_msg++;

	while(*incoming_msg!='!' && *incoming_msg!='\0') {
		Rnick[k]=*incoming_msg;
		incoming_msg++;
		k++;
	}
	Rnick[k]='\0';

	if (*Rnick != '\0') return TRUE;
	else return FALSE;
}


// replace '[' and ']' from nick/channel with '~' (else problems with .ini files)
void FixContactName(char *contactName)
{
	while(*contactName != '\0')
	{
		if((*contactName == '[') || (*contactName == ']')) *contactName='~';
		contactName++;
	}
}


void memXOR(unsigned char *s1, const unsigned char *s2, int n)
{
	while(n--) *s1++ ^= *s2++;
}


char *strfcpy(unsigned char *dest, char *buffer)		// removes leading and trailing blanks from string
{
	int i=0, k=strlen(buffer);

	while(buffer[i]==' ') i++;
	while(buffer[k-1]==' ') k--;

	buffer[k]=0;

	strcpy(dest, buffer+i);
	return dest;
}

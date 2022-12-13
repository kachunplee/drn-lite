#ifndef __DGAUTH_H__
#define __DGAUTH_H__

#include <md5.h>
#define MAXUSERNAME     17

#define MD5_LEN         16
#define DIGEST_LEN      (MAXUSERNAME+MD5_LEN)
#define DIGEST_ASC_LEN  ((DIGEST_LEN*4+2)/3)

const int MDBUFLEN = DIGEST_LEN;
const int DIGBUFLEN = DIGEST_ASC_LEN+1;

extern char * gethashpwd(const char *, const char *);
extern char * md_digest (unsigned char *, char *, const char *);
extern void MD5root(MD5_CTX *, const char *, const char *, const char *);
extern void MD5cmd(MD5_CTX *, unsigned char *, const char *);

#endif __DGAUTH_H__

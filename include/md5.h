#ifndef __MD5_H__
#define __MD5_H__

#include <stdio.h>
#include <string.h>

#define MD5_LONG unsigned long
#define MD5_CBLOCK    64
#define MD5_LBLOCK    (MD5_CBLOCK/4)
#define MD5_DIGEST_LENGTH 16
 
#define MD32_REG_T long
#define DATA_ORDER_IS_LITTLE_ENDIAN
#define INIT_DATA_A (unsigned long)0x67452301L
#define INIT_DATA_B (unsigned long)0xefcdab89L
#define INIT_DATA_C (unsigned long)0x98badcfeL
#define INIT_DATA_D (unsigned long)0x10325476L
 
#define ROTATE(a,n)     (((a)<<(n))|(((a)&0xffffffff)>>(32-(n))))
#define HOST_c2l(c,l)    (l =(((unsigned long)(*((c)++)))    ),    \
             l|=(((unsigned long)(*((c)++)))<< 8),    \
             l|=(((unsigned long)(*((c)++)))<<16),    \
             l|=(((unsigned long)(*((c)++)))<<24)    )
#define HOST_l2c(l,c)	(*((c)++)=(unsigned char)(((l)    )&0xff),    \
             *((c)++)=(unsigned char)(((l)>> 8)&0xff),    \
             *((c)++)=(unsigned char)(((l)>>16)&0xff),    \
             *((c)++)=(unsigned char)(((l)>>24)&0xff),    \
             l)
#define	HASH_MAKE_STRING(c,s)    do {    \
    unsigned long ll;        \
    ll=(c)->A; (void)HOST_l2c(ll,(s)); \
    ll=(c)->B; (void)HOST_l2c(ll,(s)); \
    ll=(c)->C; (void)HOST_l2c(ll,(s)); \
    ll=(c)->D; (void)HOST_l2c(ll,(s)); \
    } while (0)
 
#define	F(b,c,d)    ((((c) ^ (d)) & (b)) ^ (d))
#define	G(b,c,d)    ((((b) ^ (c)) & (d)) ^ (c))
#define	H(b,c,d)    ((b) ^ (c) ^ (d))
#define	I(b,c,d)    (((~(d)) | (b)) ^ (c))
 
#define R0(a,b,c,d,k,s,t) { \
    a+=((k)+(t)+F((b),(c),(d))); \
    a=ROTATE(a,s); \
    a+=b; };
#define R1(a,b,c,d,k,s,t) { \
    a+=((k)+(t)+G((b),(c),(d))); \
    a=ROTATE(a,s); \
    a+=b; };
 
#define R2(a,b,c,d,k,s,t) { \
    a+=((k)+(t)+H((b),(c),(d))); \
    a=ROTATE(a,s); \
    a+=b; };
 
#define R3(a,b,c,d,k,s,t) { \
    a+=((k)+(t)+I((b),(c),(d))); \
    a=ROTATE(a,s); \
    a+=b; };
 
typedef struct MD5state_st1{
    MD5_LONG A,B,C,D;
    MD5_LONG Nl,Nh;
    MD5_LONG data[MD5_LBLOCK];
    unsigned int num;
}MD5_CTX;
 

 
int MD5_Init(MD5_CTX *c);
 
 
void md5_block_data_order(MD5_CTX *c, const void *data_, size_t num);
 
int MD5_Update(MD5_CTX *c, const void *data_, size_t len);
 
int MD5_Final(unsigned char *md, MD5_CTX *c);

void OPENSSL_cleanse(void *ptr, size_t len);
 
unsigned char *MD5(const unsigned char *d, size_t n, unsigned char *md);

#endif


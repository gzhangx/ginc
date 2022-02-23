//	Program:	gmd5.h
//	DESC:		implement md5 protocal functions
//	Author:		Copied from RFC
//	Date:		Apr 16, 1998
//	Revisions:
//			Sep 16, 1998:	Gang Zhang revisit #1

#ifndef GANGMD5_HEADER_FILE 
#define GANGMD5_HEADER_FILE 

#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif


#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <string>


//A.2 md5.h
// MD5 context.
const int gCMD5_DIGEST_LEN=16;
class GANG_DLL_EXPORT gCMD5{

typedef unsigned char *POINTER;
typedef unsigned short int UINT2;
typedef unsigned long int UINT4;
public:
	//res got to be at least 17 bytes long (16+1)
gCMD5(char *str,char *res);
gCMD5(char *str,unsigned char *res);
gCMD5();
const unsigned char * getDigest() const {return digest;}
const std::string getDigestStr() const ;
// MD5 initialization. Begins an MD5 operation, writing a new context.
//
void MD5Init ();
// MD5 block update operation. Continues an MD5 message-digest
//  operation, processing another message block, and updating the
//  context.
void MD5Update (unsigned char * input,unsigned int  inputLen);

// MD5 finalization. Ends an MD5 message-digest operation, writing the
// the message digest and zeroizing the context.
void MD5Final (void);

typedef struct {
  UINT4 state[4];			//state (ABCD)
  UINT4 count[2];			//number of bits, modulo 2^64 (lsb first)
  unsigned char buffer[64];	//input buffer
} MD5_CTX;

//all 3 functions uses contest
protected:
unsigned char digest[gCMD5_DIGEST_LEN];	//used by MD5Final
MD5_CTX context;
//void MD5Init(void);
//void MD5Update(unsigned char *, unsigned int);
//void MD5Final (void);		//use digest

//void MD5Transform(UINT4 [4], unsigned char [64]);
//void Encode(unsigned char *, UINT4 *, int);
//void Decode(UINT4 *, unsigned char *, int);

// Constants for MD5Transform routine.

// F, G, H and I are basic MD5 functions.
UINT4 F(UINT4 x,UINT4 y,UINT4 z){return (((x) & (y)) | ((~x) & (z)));}
UINT4 G(UINT4 x,UINT4 y,UINT4 z){return (((x) & (z)) | ((y) & (~z)));}
UINT4 H(UINT4 x,UINT4 y,UINT4 z){return ((x) ^ (y) ^ (z));}
UINT4 I(UINT4 x,UINT4 y,UINT4 z){return ((y) ^ ((x) | (~z)));}

//#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
//#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
//#define H(x, y, z) ((x) ^ (y) ^ (z))
//#define I(x, y, z) ((y) ^ ((x) | (~z)))

// ROTATE_LEFT rotates x left n bits.
UINT4 ROTATE_LEFT(UINT4 x,int n){return (((x) << (n)) | ((x) >> (32-(n))));}
//#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

// FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
// Rotation is separate from addition to prevent recomputation.
//
void FF(UINT4 &a,UINT4 b,UINT4 c,UINT4 d,UINT4 x,UINT4 s,UINT4 ac);

void GG(UINT4 &a,UINT4 b,UINT4 c,UINT4 d,UINT4 x,UINT4 s,UINT4 ac);

void HH(UINT4 &a,UINT4 b,UINT4 c,UINT4 d,UINT4 x,UINT4 s,UINT4 ac);

void II(UINT4 &a,UINT4 b,UINT4 c,UINT4 d,UINT4 x,UINT4 s,UINT4 ac);



// MD5 basic transformation. Transforms state based on block.
void MD5Transform (UINT4 state[4],unsigned char block[64]);

// Encodes input (UINT4) into output (unsigned char). Assumes len is
//  a multiple of 4.
//
void Encode (unsigned char *output,UINT4 * input, int  len);

// Decodes input (unsigned char) into output (UINT4). Assumes len is
//  a multiple of 4.
//
void Decode (UINT4 *output,unsigned char * input,int  len);

};	//end of class MD5

#endif
/*
//A.4 mddriver.c

// The following makes MD default to MD5 if it has not already been
//  defined with C compiler flags.
//
#include <iostream.h>
#include <string.h>
//#include "md5.h"

void MDTestSuite(void);

int main (int argc,char ** argv)
{
	MDTestSuite ();
	return (0);
}

// Prints a message digest in hexadecimal.
//
void MDPrint (char digest[16])
{
	unsigned int i;

	for (i = 0; i < 16; i++)
		printf ("%02x", (unsigned char)digest[i]);
}

// Digests a string and prints the result.
//
void MDString (char *string)
{
	char digest[16];
	MD5 md5(string,digest);
	
	MDPrint (digest);
	printf ("\n");
}




// Digests a reference suite of strings and prints the results.
void MDTestSuite (void)
{
  printf ("MD5 test suite:\n");

  MDString ("");
  MDString ("a");
  MDString ("abc");
  MDString ("message digest");
  MDString ("abcdefghijklmnopqrstuvwxyz");
  MDString
 ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
  MDString
 ("1234567890123456789012345678901234567890\
1234567890123456789012345678901234567890");
}
*/
/*
A.5 Test suite

   The MD5 test suite (driver option "-x") should print the following
   results:

MD5 test suite:
MD5 ("") = d41d8cd98f00b204e9800998ecf8427e
MD5 ("a") = 0cc175b9c0f1b6a831c399e269772661
MD5 ("abc") = 900150983cd24fb0d6963f7d28e17f72
MD5 ("message digest") = f96b697d7cb7938d525a2f31aaf161d0
MD5 ("abcdefghijklmnopqrstuvwxyz") = c3fcd3d76192e4007dfb496cca67e13b
MD5 ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789") =
d174ab98d277d9f5a5611c2c9f419d9f
MD5 ("123456789012345678901234567890123456789012345678901234567890123456
78901234567890") = 57edf4a22be3c955ac49da2e2107b67a
*/

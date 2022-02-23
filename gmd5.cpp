//	Program:	gmd5.cpp
//	DESC:		implement md5 protocal functions
//	Author:		Copied from RFC
//	Date:		Apr 16, 1998
//	Revisions:
//			Sep 16, 1998:	Gang Zhang revisit #1



#include "gmd5.h"
using namespace std;


//A.2 md5.h
// MD5 context.
	//res got to be at least 17 bytes long (16+1)
gCMD5::gCMD5(char *str,char *res){
	unsigned int len = (unsigned int)strlen (str);
	MD5Init ();
	MD5Update ((unsigned char*)str, len);
	MD5Final();
	memcpy(res,digest,gCMD5_DIGEST_LEN);
	res[gCMD5_DIGEST_LEN]=0;
}
gCMD5::gCMD5(char *str,unsigned char *res){
	gCMD5(str,(char*)res);
}

gCMD5::gCMD5(){}

const string gCMD5::getDigestStr() const {
	char buf[33];
	for(int i=0;i<gCMD5_DIGEST_LEN;i++){
		sprintf(buf+(i*2),"%02x",digest[i]);
	}
	buf[32]=0;
	return string(buf);
}


//all 3 functions uses contest

//void MD5Init(void);
//void MD5Update(unsigned char *, unsigned int);
//void MD5Final (void);		//use digest

//void MD5Transform(UINT4 [4], unsigned char [64]);
//void Encode(unsigned char *, UINT4 *, int);
//void Decode(UINT4 *, unsigned char *, int);

// Constants for MD5Transform routine.
enum{
	S11=7,
	S12=12,
	S13=17,
	S14=22,
	S21=5,
	S22=9,
	S23=14,
	S24=20,
	S31=4,
	S32=11,
	S33=16,
	S34=23,
	S41=6,
	S42=10,
	S43=15,
	S44=21
};


// FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
// Rotation is separate from addition to prevent recomputation.
//
void gCMD5::FF(UINT4 &a,UINT4 b,UINT4 c,UINT4 d,UINT4 x,UINT4 s,UINT4 ac) {
	(a) += F ((b), (c), (d)) + (x) + (UINT4)(ac);
	(a) = ROTATE_LEFT ((a), (s));
	(a) += (b);
}

void gCMD5::GG(UINT4 &a,UINT4 b,UINT4 c,UINT4 d,UINT4 x,UINT4 s,UINT4 ac) {
	(a) += G ((b), (c), (d)) + (x) + (UINT4)(ac);
	(a) = ROTATE_LEFT ((a), (s));
	(a) += (b);
}

void gCMD5::HH(UINT4 &a,UINT4 b,UINT4 c,UINT4 d,UINT4 x,UINT4 s,UINT4 ac) {
	(a) += H ((b), (c), (d)) + (x) + ac;
	(a) = ROTATE_LEFT ((a), (s));
	(a) += (b);
}

void gCMD5::II(UINT4 &a,UINT4 b,UINT4 c,UINT4 d,UINT4 x,UINT4 s,UINT4 ac) {
	(a) += I ((b), (c), (d)) + (x) + (UINT4)(ac);
	(a) = ROTATE_LEFT ((a), (s));
	(a) += (b);
}

// MD5 initialization. Begins an MD5 operation, writing a new context.
//
void gCMD5::MD5Init ()
{
	context.count[0] = context.count[1] = 0;
	// Load magic initialization constants.
	context.state[0] = 0x67452301;
	context.state[1] = 0xefcdab89;
	context.state[2] = 0x98badcfe;
	context.state[3] = 0x10325476;
}

// MD5 block update operation. Continues an MD5 message-digest
//  operation, processing another message block, and updating the
//  context.
void gCMD5::MD5Update (unsigned char * input,unsigned int  inputLen)
{
	unsigned int i, index, partLen;

	// Compute number of bytes mod 64
	index = (unsigned int)((context.count[0] >> 3) & 0x3F);

	// Update number of bits
	if ((context.count[0] += ((UINT4)inputLen << 3))
		< ((UINT4)inputLen << 3))
		context.count[1]++;
	context.count[1] += ((UINT4)inputLen >> 29);

	partLen = 64 - index;

	// Transform as many times as possible.
	if (inputLen >= partLen) {
		memcpy(&context.buffer[index],input, partLen);
		MD5Transform (context.state, context.buffer);

		for (i = partLen; i + 63 < inputLen; i += 64)
			MD5Transform (context.state, &input[i]);
		index = 0;
	}else
		i = 0;

	// Buffer remaining input
	memcpy(&context.buffer[index],&input[i],inputLen-i);
}

// MD5 finalization. Ends an MD5 message-digest operation, writing the
// the message digest and zeroizing the context.
void gCMD5::MD5Final (void)
{
	unsigned char bits[8];
	unsigned int index, padLen;
	unsigned char PADDING[64] = {
		0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	// Save number of bits
	Encode (bits, context.count, 8);

	// Pad out to 56 mod 64.
	index = (unsigned int)((context.count[0] >> 3) & 0x3f);
	padLen = (index < 56) ? (56 - index) : (120 - index);
	MD5Update (PADDING, padLen);

	// Append length (before padding)
	MD5Update (bits, 8);
	// Store state in digest
	Encode (digest, context.state, 16);

	// Zeroize sensitive information.
	memset (&context, 0, sizeof (context));
}

// MD5 basic transformation. Transforms state based on block.
void gCMD5::MD5Transform (UINT4 state[4],unsigned char block[64])
{
	UINT4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];

	Decode (x, block, 64);

	// Round 1
	FF (a, b, c, d, x[ 0], S11, 0xd76aa478); // 1
	FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); // 2
	FF (c, d, a, b, x[ 2], S13, 0x242070db); // 3
	FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); // 4
	FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); // 5
	FF (d, a, b, c, x[ 5], S12, 0x4787c62a); // 6
	FF (c, d, a, b, x[ 6], S13, 0xa8304613); // 7
	FF (b, c, d, a, x[ 7], S14, 0xfd469501); // 8
	FF (a, b, c, d, x[ 8], S11, 0x698098d8); // 9 
	FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); // 10
	FF (c, d, a, b, x[10], S13, 0xffff5bb1); // 11
	FF (b, c, d, a, x[11], S14, 0x895cd7be); // 12
	FF (a, b, c, d, x[12], S11, 0x6b901122); // 13
	FF (d, a, b, c, x[13], S12, 0xfd987193); // 14
	FF (c, d, a, b, x[14], S13, 0xa679438e); // 15
	FF (b, c, d, a, x[15], S14, 0x49b40821); // 16

	// Round 2
	GG (a, b, c, d, x[ 1], S21, 0xf61e2562); // 17
	GG (d, a, b, c, x[ 6], S22, 0xc040b340); // 18
	GG (c, d, a, b, x[11], S23, 0x265e5a51); // 19
	GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); // 20
	GG (a, b, c, d, x[ 5], S21, 0xd62f105d); // 21
	GG (d, a, b, c, x[10], S22,  0x2441453); // 22
	GG (c, d, a, b, x[15], S23, 0xd8a1e681); // 23
	GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); // 24
	GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); // 25
	GG (d, a, b, c, x[14], S22, 0xc33707d6); // 26
	GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); // 27
	GG (b, c, d, a, x[ 8], S24, 0x455a14ed); // 28
	GG (a, b, c, d, x[13], S21, 0xa9e3e905); // 29
	GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); // 30
	GG (c, d, a, b, x[ 7], S23, 0x676f02d9); // 31
	GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); // 32

	// Round 3
	HH (a, b, c, d, x[ 5], S31, 0xfffa3942); // 33
	HH (d, a, b, c, x[ 8], S32, 0x8771f681); // 34
	HH (c, d, a, b, x[11], S33, 0x6d9d6122); // 35
	HH (b, c, d, a, x[14], S34, 0xfde5380c); // 36
	HH (a, b, c, d, x[ 1], S31, 0xa4beea44); // 37
	HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); // 38
	HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); // 39
	HH (b, c, d, a, x[10], S34, 0xbebfbc70); // 40
	HH (a, b, c, d, x[13], S31, 0x289b7ec6); // 41
	HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); // 42
	HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); // 43
	HH (b, c, d, a, x[ 6], S34,  0x4881d05); // 44
	HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); // 45
	HH (d, a, b, c, x[12], S32, 0xe6db99e5); // 46
	HH (c, d, a, b, x[15], S33, 0x1fa27cf8); // 47
	HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); // 48

	  // Round 4
	II (a, b, c, d, x[ 0], S41, 0xf4292244); // 49
	II (d, a, b, c, x[ 7], S42, 0x432aff97); // 50
	II (c, d, a, b, x[14], S43, 0xab9423a7); // 51
	II (b, c, d, a, x[ 5], S44, 0xfc93a039); // 52
	II (a, b, c, d, x[12], S41, 0x655b59c3); // 53
	II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); // 54
	II (c, d, a, b, x[10], S43, 0xffeff47d); // 55
	II (b, c, d, a, x[ 1], S44, 0x85845dd1); // 56
	II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); // 57
	II (d, a, b, c, x[15], S42, 0xfe2ce6e0); // 58
	II (c, d, a, b, x[ 6], S43, 0xa3014314); // 59
	II (b, c, d, a, x[13], S44, 0x4e0811a1); // 60
	II (a, b, c, d, x[ 4], S41, 0xf7537e82); // 61
	II (d, a, b, c, x[11], S42, 0xbd3af235); // 62
	II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); // 63
	II (b, c, d, a, x[ 9], S44, 0xeb86d391); // 64

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;

	// Zeroize sensitive information.
	memset ((POINTER)x, 0, sizeof (x));
}

// Encodes input (UINT4) into output (unsigned char). Assumes len is
//  a multiple of 4.
//
void gCMD5::Encode (unsigned char *output,UINT4 * input, int  len)
{
	int i, j;
	for (i = 0, j = 0; j < len; i++, j += 4) {
		output[j] = (unsigned char)(input[i] & 0xff);
		output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
		output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
		output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
	}
}

// Decodes input (unsigned char) into output (UINT4). Assumes len is
//  a multiple of 4.
//
void gCMD5::Decode (UINT4 *output,unsigned char * input,int  len)
{
	int i, j;
	for (i = 0, j = 0; j < len; i++, j += 4)
		output[i] = ((UINT4)input[j]) | (((UINT4)input[j+1]) << 8) |
			(((UINT4)input[j+2]) << 16) | (((UINT4)input[j+3]) << 24);
}

	//end of class MD5


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

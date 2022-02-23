/********************************************/
/*  SIXPACK.h -- Data compression program   */
/*  Written by Philip G. Gage, April 1991   */
/*  Converted to C++ on Feb 22, 1999        */
/********************************************/


#include <stdio.h>
#include <string.h>

#ifndef SIXPACK_COMPRESS_HEADER_FILE
#define SIXPACK_COMPRESS_HEADER_FILE

#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif

//abstrut class for input and output streams
class CXFileStream{
public:
	virtual int mgetc()=0;
	virtual int mputc(int c)=0;
};

class GANG_DLL_EXPORT CSixPack{
	short *head, *tail;       // Hash table
	short *succ, *pred;       // Doubly linked lists
	unsigned char *buffer;    // Text buffer

	// Define hash key function using MINCOPY characters of string prefix
	inline int getkey(int n) {return ((buffer[n] ^ (buffer[(n+1)%maxsize]<<4) ^ (buffer[(n+2)%maxsize]<<8)) & HASHMASK);}

	// Adaptive Huffman variables

	//*** Bit packing routines ***

	int input_bit_count;// = 0;           // Input bits buffered *
	int input_bit_buffer;// = 0;          // Input buffer *
	int output_bit_count;// = 0;          // Output bits buffered *
	int output_bit_buffer;// = 0;         // Output buffer *
	long bytes_in,// = 0, 
	bytes_out;// = 0;  // File size counters *


	enum{
		TEXTSEARCH= 1000,   // Max strings to search in text file
		BINSEARCH =  200,   // Max strings to search in binary file
		TEXTNEXT  =   50,   // Max search at next character in text file
		BINNEXT   =   20,   // Max search at next character in binary file
		MAXFREQ   = 2000,   // Max frequency count before table reset
		MINCOPY   =    3,   // Shortest string copy length
		MAXCOPY   =   64,   // Longest string copy length
		SHORTRANGE=    3,   // Max distance range for shortest length copy
		COPYRANGES=    6,   // Number of string copy distance bit ranges

		CODESPERRANGE=(MAXCOPY - MINCOPY + 1),
		NIL=-1,                    // End of linked list marker
		HASHSIZE=16384,            // Number of entries in hash table
		HASHMASK=(HASHSIZE - 1),   // Mask for hash key wrap

		TERMINATE=256,             // EOF code
		FIRSTCODE=257,             // First code for copy lengths
		GSXMAXCHAR=(FIRSTCODE+COPYRANGES*CODESPERRANGE-1),
		SUCCMAX=(GSXMAXCHAR+1),
		TWICEMAX=(2*GSXMAXCHAR+1),
		ROOT=1
	};
	int copymin[COPYRANGES], copymax[COPYRANGES];
	int maxdistance, maxsize;
	int distance, insert,	//= MINCOPY,
			dictfile,	// = 0,
			binary;		// = 0;
	static short copybits[COPYRANGES];// = {4,6,8,10,12,14};   // Distance bits

	short left[GSXMAXCHAR+1], right[GSXMAXCHAR+1];  // Huffman tree
	short up[TWICEMAX+1], freq[TWICEMAX+1];


//* Write one bit to output file *
void output_bit(CXFileStream *output,int bit);

// Read a bit from input file *
int input_bit(CXFileStream *input);

//* Write multibit code to output file *
void output_code(CXFileStream *output,int code,int bits);

//* Read multibit code from input file *
int input_code(CXFileStream *input,int bits);

//* Flush any remaining bits to output file before closing file *
void flush_bits(CXFileStream *output);

//*** Adaptive Huffman frequency compression ***

//* Data structure based partly on "Application of Splay Trees
//   to Data Compression", Communications of the ACM 8/88 *

//* Initialize data for compression or decompression *
void initialize();

//* Update frequency counts from leaf to root *
void update_freq(int a,int b);

//* Update Huffman model for each character code *
void update_model(int code);

//* Compress a character code to output stream *
void compress(CXFileStream *output,int code);

//* Uncompress a character code from input stream *
int uncompress(CXFileStream *input);

//*** Hash table linked list string search routines ***

//* Add node to head of list *
void add_node(int n);

//* Delete node from tail of list *
void delete_node(int n);

//* Find longest string matching lookahead buffer string *
int match(int n,int depth);

//*** Finite Window compression routines ***

//* Check first buffer for ordered dictionary file *
//* Better compression using short distance copies *
void dictionary();

enum PROCESSING_STATE{
IDLE=0,    //* Not processing a copy *
COPY=1    //* Currently processing copy *
};

public:
//* Encode file from input to output *
void encode(CXFileStream *input,CXFileStream *output);

//* Decode file from input to output *
void decode(CXFileStream *input,CXFileStream *output);

inline int get_bytes_out(){
	return bytes_out;
}

CSixPack();

void free_all();

~CSixPack();

};

/*
class CXFileStream{
public:
	FILE *m_fp;
	void setfp(FILE *fp){
		m_fp=fp;
	}
	virtual int mgetc(){
		return fgetc(m_fp);
	}
	virtual int mputc(int c){
		return fputc(c,m_fp);
	}
};


//* Main program *
void main(int argc,char **argv)
{
  FILE *infile, *outfile;
   CXFileStream infp,outfp;
  if (argc < 3 || argc > 4)
    printf("Usage: %s inputfile outputfile [decompress]\n",argv[0]);
  else if (!strcmp(argv[1],argv[2]))
    printf("File names must be different\n");
  else if ((infile = fopen(argv[1],"rb")) == NULL)
    printf("Error opening input file %s\n",argv[1]);
  else if ((outfile = fopen(argv[2],"wb")) == NULL)
    printf("Error opening output file %s\n",argv[2]);
  else {
    if (argc == 3) {
	  infp.setfp(infile);
	  outfp.setfp(outfile);
      encode(&infp,&outfp);
      printf("Packed from %ld bytes to %ld bytes\n",bytes_in,bytes_out);
    } else {
		infp.setfp(infile);
		outfp.setfp(outfile);
		decode(&infp,&outfp);
		printf("Unpacked from %ld bytes to %ld bytes\n",bytes_in,bytes_out);
    }
    fclose(outfile); fclose(infile);
  }
}
*/

class gCSixPackStream:public CXFileStream{
	unsigned char *m_buf;
	int m_size;
	int pos;
public:
	gCSixPackStream(unsigned char *buf,int size):pos(0),m_buf(buf),m_size(size){
	}
	int mgetc(){
		if(pos==m_size)
			return EOF;
		return m_buf[pos++];
	}
	int mputc(int c){
		if(pos==m_size)
			throw "Compress: len >nolen";
			//return EOF;	//should throw???!!!
		m_buf[pos++]=(c&0xff);
		return c;
	}
};


#endif
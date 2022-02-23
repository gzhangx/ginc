/********************************************/
/*  SIXPACK.C -- Data compression program   */
/*  Written by Philip G. Gage, April 1991   */
/********************************************/

#include <sixpack.h>

#include <stdio.h>
#include <malloc.h>
#include <string.h>



//* Write one bit to output file *
void CSixPack::output_bit(CXFileStream *output,int bit)
{
  output_bit_buffer <<= 1;
  if (bit) output_bit_buffer |= 1;
  if (++output_bit_count == 8) {
    output->mputc(output_bit_buffer);
    output_bit_count = 0;
    ++bytes_out;
  }
}

// Read a bit from input file *
int CSixPack::input_bit(CXFileStream *input)
{
  int bit;

  if (input_bit_count-- == 0) {
    input_bit_buffer = input->mgetc();
    if (input_bit_buffer == EOF) {
      throw (" UNEXPECTED END OF FILE");
    }
    ++bytes_in;
    input_bit_count = 7;
  }
  bit = (input_bit_buffer & 0x80) != 0;
  input_bit_buffer <<= 1;
  return(bit);
}

//* Write multibit code to output file *
void CSixPack::output_code(CXFileStream *output,int code,int bits)
{
  int i;

  for (i = 0; i<bits; i++) {
    output_bit(output,code & 0x01);
    code >>= 1;
  }
}

//* Read multibit code from input file *
int CSixPack::input_code(CXFileStream *input,int bits)
{
  int i, bit = 1, code = 0;

  for (i = 0; i<bits; i++) {
    if (input_bit(input)) code |= bit;
    bit <<= 1;
  }
  return(code);
}

//* Flush any remaining bits to output file before closing file *
void CSixPack::flush_bits(CXFileStream *output)
{
  if (output_bit_count > 0) {
    output->mputc((output_bit_buffer << (8-output_bit_count)));
    ++bytes_out;
  }
}

//*** Adaptive Huffman frequency compression ***

//* Data structure based partly on "Application of Splay Trees
//   to Data Compression", Communications of the ACM 8/88 *

//* Initialize data for compression or decompression *
void CSixPack::initialize()
{


	input_bit_count = 0;           // Input bits buffered *
	input_bit_buffer = 0;          // Input buffer *
	output_bit_count = 0;          // Output bits buffered *
	output_bit_buffer = 0;         // Output buffer *
	bytes_in= 0;
	bytes_out=0;

	insert = MINCOPY;
	dictfile = 0;
	binary = 0;

  int i, j;

  //* Initialize Huffman frequency tree *
  for (i = 2; i<=TWICEMAX; i++) {
    up[i] = i/2;
    freq[i] = 1;
  }
  for (i = 1; i<=GSXMAXCHAR; i++) {
    left[i] = 2*i;
    right[i] = 2*i+1;
  }

  //* Initialize copy distance ranges *
  j = 0;
  for (i = 0; i<COPYRANGES; i++) {
    copymin[i] = j;
    j += 1 << copybits[i];
    copymax[i] = j - 1;
  }
  maxdistance = j - 1;
  maxsize = maxdistance + MAXCOPY;
}

//* Update frequency counts from leaf to root *
void CSixPack::update_freq(int a,int b)
{
  do {
    freq[up[a]] = freq[a] + freq[b];
    a = up[a];
    if (a != ROOT) {
      if (left[up[a]] == a) b = right[up[a]];
      else b = left[up[a]];
    }
  } while (a != ROOT);

  //* Periodically scale frequencies down by half to avoid overflow *
  //* This also provides some local adaption and better compression *
  if (freq[ROOT] == MAXFREQ)
    for (a = 1; a<=TWICEMAX; a++) freq[a] >>= 1;
}

//* Update Huffman model for each character code *
void CSixPack::update_model(int code)
{
  int a, b, c, ua, uua;

  a = code + SUCCMAX;
  ++freq[a];
  if (up[a] != ROOT) {
    ua = up[a];
    if (left[ua] == a) update_freq(a,right[ua]);
    else update_freq(a,left[ua]);
    do {
      uua = up[ua];
      if (left[uua] == ua) b = right[uua];
      else b = left[uua];

      /* If high freq lower in tree, swap nodes */
      if (freq[a] > freq[b]) {
        if (left[uua] == ua) right[uua] = a;
        else left[uua] = a;
        if (left[ua] == a) {
          left[ua] = b; c = right[ua];
        } else {
          right[ua] = b; c = left[ua];
        }
        up[b] = ua; up[a] = uua;
        update_freq(b,c); a = b;
      }
      a = up[a]; ua = up[a];
    } while (ua != ROOT);
  }
}

//* Compress a character code to output stream *
void CSixPack::compress(CXFileStream *output,int code)
{
  int a, sp = 0;
  int stack[50];

  a = code + SUCCMAX;
  do {
    stack[sp++] = (right[up[a]] == a);
    a = up[a];
  } while (a != ROOT);
  do {
    output_bit(output,stack[--sp]);
  } while (sp);
  update_model(code);
}

//* Uncompress a character code from input stream *
int CSixPack::uncompress(CXFileStream *input)
{
  int a = ROOT;

  do {
    if (input_bit(input)) a = right[a];
    else a = left[a];
  } while (a <= GSXMAXCHAR);
  update_model(a-SUCCMAX);
  return(a-SUCCMAX);
}

//*** Hash table linked list string search routines ***

//* Add node to head of list *
void CSixPack::add_node(int n)  
{
  int key;

  key = getkey(n);
  if (head[key] == NIL) {
    tail[key] = n;
    succ[n] = NIL;
  } else {
    succ[n] = head[key];
    pred[head[key]] = n;
  }
  head[key] = n;
  pred[n] = NIL;
}

//* Delete node from tail of list *
void CSixPack::delete_node(int n)
{
  int key;

  key = getkey(n);
  if (head[key] == tail[key])
    head[key] = NIL;
  else {
    succ[pred[tail[key]]] = NIL;
    tail[key] = pred[tail[key]];
  }
}

//* Find longest string matching lookahead buffer string *
int CSixPack::match(int n,int depth)
{
  int i, j, index, key, dist, len, best = 0, count = 0;

  if (n == maxsize) n = 0;
  key = getkey(n);
  index = head[key];
  while (index != NIL) {
    if (++count > depth) break;     //* Quit if depth exceeded *
    if (buffer[(n+best)%maxsize] == buffer[(index+best)%maxsize]) {
      len = 0;  i = n;  j = index;
      while (buffer[i]==buffer[j] && len<MAXCOPY && j!=n && i!=insert) {
        ++len;
        if (++i == maxsize) i = 0;
        if (++j == maxsize) j = 0;
      }
      dist = n - index;
      if (dist < 0) dist += maxsize;
      dist -= len;
      //* If dict file, quit at shortest distance range *
      if (dictfile && dist > copymax[0]) break;
      if (len > best && dist <= maxdistance) {     //* Update best match *
        if (len > MINCOPY || dist <= copymax[SHORTRANGE+binary]) {
          best = len; distance = dist;
        }
      }
    }
    index = succ[index];
  }
  return(best);
}

//*** Finite Window compression routines ***

//* Check first buffer for ordered dictionary file *
//* Better compression using short distance copies *
void CSixPack::dictionary()
{
  int i = 0, j = 0, k, count = 0;

  //* Count matching chars at start of adjacent lines *
  while (++j < MINCOPY+MAXCOPY) {
    if (buffer[j-1] == 10) {
      k = j;
      while (buffer[i++] == buffer[k++]) ++count;
      i = j;
    }
  }
  //* If matching line prefixes > 25% assume dictionary *
  if (count > (MINCOPY+MAXCOPY)/4) dictfile = 1;
}


//* Encode file from input to output *
void CSixPack::encode(CXFileStream *input,CXFileStream *output)
{
  int c, i, n=MINCOPY, addpos=0, len=0, full=0, nextlen;
  PROCESSING_STATE state=IDLE;

  initialize();
  //head = (short*)malloc((unsigned long)HASHSIZE*sizeof(short));
  //tail = (short*)malloc((unsigned long)HASHSIZE*sizeof(short));
  //succ = (short*)malloc((unsigned long)maxsize*sizeof(short));
  //pred = (short*)malloc((unsigned long)maxsize*sizeof(short));
  //buffer = (unsigned char *) malloc(maxsize*sizeof(unsigned char));
  if (head==NULL || tail==NULL || succ==NULL || pred==NULL || buffer==NULL) {
    throw ("Error allocating memory");
  }

  //* Initialize hash table to empty *
  for (i = 0; i<HASHSIZE; i++) {
    head[i] = NIL;
  }

  //* Compress first few characters using Huffman *
  for (i = 0; i<MINCOPY; i++) {
    if ((c = input->mgetc()) == EOF) {
      compress(output,TERMINATE);
      flush_bits(output);
      //return(bytes_in);
	  return;
    }
    compress(output,c);  ++bytes_in;
    buffer[i] = c;
  }

  //* Preload next few characters into lookahead buffer *
  for (i = 0; i<MAXCOPY; i++) {
    if ((c = input->mgetc()) == EOF) break;
    buffer[insert++] = c;  ++bytes_in;
    if (c > 127) binary = 1;     //* Binary file ? *
  }
  dictionary();  //* Check for dictionary file *

  while (n != insert) {
    //* Check compression to insure really a dictionary file *
    if (dictfile && ((bytes_in % MAXCOPY) == 0))
      if (bytes_in/bytes_out < 2)
        dictfile = 0;     //* Oops, not a dictionary file ! *

    //* Update nodes in hash table lists *
    if (full) delete_node(insert);
    add_node(addpos);

    //* If doing copy, process character, else check for new copy *
    if (state == COPY) {
      if (--len == 1) state = IDLE;
    } else {

      //* Get match length at next character and current char *
      if (binary) {
        nextlen = match(n+1,BINNEXT);
        len = match(n,BINSEARCH);
      } else {
        nextlen = match(n+1,TEXTNEXT);
        len = match(n,TEXTSEARCH);
      }

      //* If long enough and no better match at next char, start copy *
      if (len >= MINCOPY && len >= nextlen) {
        state = COPY;

        //* Look up minimum bits to encode distance *
        for (i = 0; i<COPYRANGES; i++) {
          if (distance <= copymax[i]) {
            compress(output,FIRSTCODE-MINCOPY+len+i*CODESPERRANGE);
            output_code(output,distance-copymin[i],copybits[i]);
            break;
          }
        }
      }
      else   //* Else output single literal character */
        compress(output,buffer[n]);
    }

    //* Advance buffer pointers *
    if (++n == maxsize) n = 0;
    if (++addpos == maxsize) addpos = 0;

    //* Add next input character to buffer *
    if (c != EOF) {
      if ((c = input->mgetc()) != EOF) {
        buffer[insert++] = c;  ++bytes_in;
      } else full = 0;
      if (insert == maxsize) {
        insert = 0; full = 1;
      }
    }
  }

  //* Output EOF code and free memory *
  compress(output,TERMINATE);
  flush_bits(output);
  //free(head); free(tail); free(succ); free(pred);
  //free(buffer);
}

//* Decode file from input to output *
void CSixPack::decode(CXFileStream *input,CXFileStream *output)
{
  int c, i, j, k, dist, len, n = 0, index;

  initialize();
  if(buffer==NULL)
	buffer = new unsigned char[maxsize];
  if (buffer == NULL) {
    throw "Error allocating memory";
  }

  while ((c = uncompress(input)) != TERMINATE) {
    if (c < 256) {     //* Single literal character ? *
      output->mputc(c);
      ++bytes_out;
      buffer[n++] = c;
      if (n == maxsize) n = 0;
    } else {            //* Else string copy length/distance codes *
      index = (c - FIRSTCODE)/CODESPERRANGE;
      len = c - FIRSTCODE + MINCOPY - index*CODESPERRANGE;
      dist = input_code(input,copybits[index]) + len + copymin[index];
      j = n; k = n - dist;
      if (k < 0) k += maxsize;
      for (i = 0; i<len; i++) {
        output->mputc(buffer[k]);  ++bytes_out;
        buffer[j++] = buffer[k++];
        if (j == maxsize) j = 0;
        if (k == maxsize) k = 0;
      }
      n += len;
      if (n >= maxsize) n -= maxsize;
    }
  }
  //buffer will be init once
  //free(buffer);
}



CSixPack::CSixPack(){
	initialize();

	buffer=NULL;
	head=NULL;
	tail=NULL;
	succ=NULL;
	pred=NULL;

	try{
	buffer = new unsigned char [maxsize];
	if(buffer==NULL)throw "Can't allocate buffer";
	head = new short[HASHSIZE];
	if(head==NULL)throw "Can't allocate hash head";
	tail = new short[HASHSIZE];
	if(tail==NULL)throw "Can't allocate hash tail";
	succ = new short[maxsize];
	if(succ==NULL)throw "Can't allocate succ";
	pred = new short[maxsize];
	if(pred==NULL)throw "Can't allocate pred";
	}catch(char *p){
		free_all();
		throw p;
	}
}

void CSixPack::free_all(){
	if(buffer)
		delete(buffer);
	if(head)
		delete head;
	if(tail)
		delete tail;
	if(succ)
		delete succ;
	if(pred)
		delete pred;

	buffer=NULL;
	head=NULL;
	tail=NULL;
	succ=NULL;
	pred=NULL;
}

CSixPack::~CSixPack(){
	free_all();
}

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

short CSixPack::copybits[COPYRANGES] = {4,6,8,10,12,14};   // Distance bits
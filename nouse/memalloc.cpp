//	Program:	memalloc.cpp
//	DESC:		implement operator new to track un freed
//				or improper deleted memory
//	Author:		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Date:		Apr 16, 1998
//
//	Use last_report() to report memory usage

#include <time.h>
#include "locksem.h"
//use last report to report memory usage
void last_report();

enum{
	MEM_EXTRA=32,			//bytes will be extraly allocated for verify
	NEW_MALLOC_MAX=4096
};

class gmalloc_struct{

	gCLockSem malloc_sem;
public:
	struct malloc_store{
		void *ptr;
		time_t time;
		int size;
		int file_line;
		const char *fname;
    int which;
	}m_stored[NEW_MALLOC_MAX];

	int m_totalmallocs;
	int m_totalfrees;
	int m_num_malloced;
	int m_memallocated;
	int m_peak_mem;
	int m_failed;
	gmalloc_struct():m_totalmallocs(0),
		m_totalfrees(0),m_num_malloced(0),m_peak_mem(0),
		m_memallocated(0),m_failed(0)
	{
		memset(m_stored,0,NEW_MALLOC_MAX*sizeof(malloc_store));
	}

	protected:
	int insert(void *ptr,int size,const char *fname,int line){
    static int staticPos=0;
		for(int i=0;i<NEW_MALLOC_MAX;i++){
			if(m_stored[i].ptr==NULL){
				m_stored[i].ptr=ptr;
				m_stored[i].size=size;
				m_stored[i].file_line=line;
				m_stored[i].fname=fname;
        m_stored[i].which=staticPos++;
				time(&m_stored[i].time);
				return i;
			}
		}
		return -1;
	}

	int find_ptr(void *ptr){
		for(int i=0;i<NEW_MALLOC_MAX;i++){
			if(m_stored[i].ptr==ptr){
				return i;
			}
		}
		return -1;
	}

	public:
	void *add_alloc(int size,const char *fname=NULL,int line=0){
		malloc_sem.GWait();
		void *ptr=malloc(size+MEM_EXTRA);
		if(ptr!=NULL){
			if(insert(ptr,size,fname,line)==-1){
				free(ptr);
				m_failed++;
				malloc_sem.GSignal();
				return NULL;
			}
			m_totalmallocs++;
			m_num_malloced++;
			m_memallocated+=size;
			if(m_memallocated>m_peak_mem)m_peak_mem=m_memallocated;
			memset((char*)ptr+size,0,MEM_EXTRA);
		}else{
			m_failed++;
		}

		malloc_sem.GSignal();
		return ptr;
	}

	void del_free(void *ptr){
		malloc_sem.GWait();
		int ind=find_ptr(ptr);
		if(ind==-1){
			malloc_sem.GSignal();
			::MessageBox(NULL,"free invalidate memory","memerr",MB_OK);
			return;
		}
		m_totalfrees++;
		m_num_malloced--;
		m_stored[ind].ptr=NULL;

		int size=m_stored[ind].size;
		m_memallocated-=size;
		for(int i=0;i<MEM_EXTRA;i++){
			if( *((unsigned char*)ptr+size+i)!=0){
				::MessageBox(NULL,"memory corrupted!","memerr",MB_OK);
				//TRACE("%s(%i) : %p len %i\n",
				//m_stored[ind].fname,
				//m_stored[ind].file_line,
				//m_stored[ind].ptr,
				//m_stored[ind].size
				//);
				break;
			}
		}
		free(ptr);
		malloc_sem.GSignal();
	}
};

gmalloc_struct gmalloc_storage;
void *operator new(unsigned int size){
	return gmalloc_storage.add_alloc(size);
}

void *operator new(unsigned int size,LPCSTR lpszFileName){
	return gmalloc_storage.add_alloc(size,lpszFileName);
}

void *operator new(unsigned int size,LPCSTR lpszFileName, int nLine){
	return gmalloc_storage.add_alloc(size,lpszFileName,nLine);
}

void operator delete(void * ptr){
	gmalloc_storage.del_free(ptr);
}


void last_report(){
	//char msg[1024];
	//TRACE("total alloc %i, total free %i\n",
	//gmalloc_storage.m_totalmallocs,
	//gmalloc_storage.m_totalfrees
	//);
	//::MessageBox(NULL,msg,"report",MB_OK);

	//TRACE("num allocs %i memalloced %i\n",
	//gmalloc_storage.m_num_malloced,
	//gmalloc_storage.m_memallocated);

	//::MessageBox(NULL,msg,"report",MB_OK);

	//TRACE("peak mem %i\n",gmalloc_storage.m_peak_mem);
	//if(gmalloc_storage.m_failed){
		//TRACE("Malloc fails: %i\n",gmalloc_storage.m_failed);
	//}
	//::MessageBox(NULL,msg,"report",MB_OK);

	for(int i=0;i<NEW_MALLOC_MAX;i++){
		if(gmalloc_storage.m_stored[i].ptr!=NULL){
			//TRACE("unfreed memory %s",
			//ctime(&gmalloc_storage.m_stored[i].time));

			//TRACE("%s(%i) at %i : loc %p, %i bytes\n",
			//	gmalloc_storage.m_stored[i].fname,
			//	gmalloc_storage.m_stored[i].file_line,
      //  gmalloc_storage.m_stored[i].which,
			//	gmalloc_storage.m_stored[i].ptr,
			//	gmalloc_storage.m_stored[i].size
			//);
		}
	}

	if(gmalloc_storage.m_totalmallocs!=gmalloc_storage.m_totalfrees){
		::MessageBox(NULL,"memmngr: Memory leak found","iPPhone",MB_OK);
	}
}

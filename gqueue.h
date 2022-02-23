//////////////////////////////////////////////////////////////
// File:	gqueue.h
// Author:	Gang Zhang (gzhang@icx.net)
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Date:		04/12/1996
// Desc:	a generic thread safe queue.
// Note:	lock has been move from gwinlock_queue to gen_queue,
//			because in certain muti thread program, if derive from
//			gen_winlock_queue, which derive from gqueue (and gqueue's
//			virtual lock=0, if winlock queue is decleared like:
//			class something public thread{
//				winlock_queue q;
//				................
//				InitInstance(){
//					while(alive){}
//					//not alive now
//					q.clear()//which uses lock, and since thread is 
//					deleted now, q's winlock is gone and lock is 
//					purevirtual now....
//				}
//			To treat that, q need to be newed or not derived
//////////////////////////////////////////////////////////////
#ifndef GENERIC_TEMPLATE_T_S_QUEUE_HEADER
#define GENERIC_TEMPLATE_T_S_QUEUE_HEADER
//////////////////////////////////////////////////////////////

///////////////////////// start of gen_queue /////////////////

#include <gerror.h>
#include <locksem.h>

template<class T>
class queue_entry{
	public:
		T *val;
		queue_entry(T *eval):val(eval){}
		~queue_entry(){}
		queue_entry *prev;
		queue_entry *next;
};

template<class T>
class gen_queue{
	gCLockSem gen_winlock_csem;
public:
protected:
	queue_entry<T> *head;
	queue_entry<T> *tail;
	unsigned long count;

	//throw exception on semphore error

	//virtual void lock()=0;
	//virtual void unlock()=0;

	virtual void count_inc(){
		count++;
	}
	void count_dec(){
		count--;
	}

public:

	void lock(){
		gen_winlock_csem.GWait();
	}
	void unlock(){
		gen_winlock_csem.GSignal();
	}

	unsigned long get_count(){
		int ct;
		lock();
		ct=count;
		unlock();
		return ct;
	}

	gen_queue(void){
		count=0;
		head=tail=NULL;
	}
	
	virtual ~gen_queue()
	{
		//clear(); since clear() means this->clear == gen_queue->clear, it is useless
		//clear() must be called to clear the queue.
	};

	queue_entry<T>* queue(T*val){
		return queue_tail(val);
	}
	
	queue_entry<T>* queue_tail(T *val){
		queue_entry<T> *element=new queue_entry<T>(val);
		if(element==NULL)return NULL;
		lock();
		count_inc();
		if(head==NULL){
			element->prev=NULL;
			element->next=NULL;
			head=tail=element;
		}else{
			element->prev=tail;
			element->next=NULL;
			tail->next=element;
			tail=element;
		}
		unlock();
		return element;
	}

	queue_entry<T>* queue_head(T *val){
		queue_entry<T> *element=new queue_entry<T>(val);
		if(element==NULL)return NULL;
		lock();
		count_inc();
		if(head==NULL){
			element->prev=NULL;
			element->next=NULL;
			head=tail=element;
		}else{
			element->prev=NULL;
			element->next=head;
			head->prev=element;
			head=element;
		}
		unlock();
		return element;
	}

	virtual T* unqueue_tail(void){
		lock();
		queue_entry<T> *tmp=tail;
		if(tail){
			count_dec();
			if(head==tail){
				head=tail=NULL;
			}else{
				tail=tail->prev;
				tail->next=NULL;
			}
			unlock();
			T *tt=tmp->val;
			delete tmp;
			return tt;
		}
		unlock();
		return NULL;
	}

	virtual T* unqueue(void){
		return unqueue_head();
	}

	virtual T* unqueue_head(void){
		lock();
		if(head){
			count_dec();
			queue_entry<T> *tmp=head;
			if(head==tail){
				head=tail=NULL;
			}else{
				head=head->next;
				head->prev=NULL;
			}
			unlock();
			T *tt=tmp->val;
			delete tmp;
			return tt;
		}
		unlock();
		return NULL;
	}

	T* unqueue_entry(queue_entry<T>* qs){
		lock();
		if(count<=0){
			unlock();
			return NULL;
			throw gerror_str("gen_queue: unqueue invalidate entry");
		}
		
		if(count==1){
			if(qs!=head){
				unlock();
				throw gerror_str("gen_queue: unqueue invalidate entry");
					return NULL;
			}
		}
		//safty check
		if(qs->prev){
			if(qs->prev->next!=qs){
				unlock();
				throw gerror_str("gen_queue: unqueue invalidate entry");
				return NULL;
			}
		}
		if(qs->next){
			if(qs->next->prev!=qs){
				unlock();
				throw gerror_str("gen_queue: unqueue invalidate entry");
				return NULL;
			}
		}
		//end of safty check

		count_dec();
		if(qs->next){
			qs->next->prev=qs->prev;
		}else
			tail=qs->prev;
		
		if(qs->prev){
			qs->prev->next=qs->next;
		}else
			head=qs->next;


		unlock();
		T* tmp=qs->val;
		qs->prev=NULL;
		qs->next=NULL;
		delete qs;
		return tmp;
	}


	virtual void Remove_val( T* val)
	{
		delete val;
	}

	void clear(void){
		T *ent;
		//this (instead of lock and delete all) will prevent lock situation
		//where on delete the object itself is trying to unqueue itself
		//from the queue.
		while( (ent=unqueue())!=NULL)delete ent;
	}
};
//////////////////////////// end of gen_queue //////////////////



template<class T>
class gen_wait_queue : public gen_queue<T>{
protected:
	gCLockSem gen_wait_csem;

	void count_inc(){
		count++;
		gen_wait_csem.GSignal();
	}

public:
	//return the handle for wait when no entries are available
	//this is the handle unqueue_wait is wait on when the above happens
	HANDLE get_wait_handle(){
		return gen_wait_csem.get_handle();
	}

	gen_wait_queue(){
		gen_wait_csem.GWait();
	}

	

	virtual T* unqueue_wait(int tm=INFINITE){
		if(gen_wait_csem.GWait(tm)==WAIT_TIMEOUT)return NULL;
		T* tmp=unqueue();
		while(tmp==NULL){
			if(gen_wait_csem.GWait(tm)==WAIT_TIMEOUT)return NULL;
			tmp=unqueue();
		}		
		gen_wait_csem.GSignal();
		return tmp;
	}
	
};


/*
class CTSLockSem{
	HANDLE hsem;
public:
	CTSLockSem(BOOL bInitialOwner=TRUE,LPCSTR lpName=NULL);

	void init(BOOL bInitialOwner,LPCSTR lpName=NULL);
	~CTSLockSem();
	void GWait();
	void GSignal();
};
*/

/*
template<class T,class S>
class gen_winlock_queue : public gen_queue<T,S>{
	CTSLockSem gen_winlock_csem;
public:

	gen_winlock_queue(){
		//gen_winlock_csem.init(TRUE,name);
	}
	~gen_winlock_queue(){
	}

	void lock(){
		gen_winlock_csem.GWait();
	}
	void unlock(){
		gen_winlock_csem.GSignal();
	}
};
*/

template<class T,class S>
class gen_find_queue: public gen_queue<T>{
protected:
	//return 1 if match
	//if string: strcmp(val,s)
	//if val is struct and s is int, val->something==s
	//revision: now val better is a class that has == of type S
	//defined
	virtual BOOL equ_val(T *val,S s){
		return (*val==s);
	}


	//internal use only.
	queue_entry<T> *find_entry(S s){
		queue_entry<T> *tmp=head;
		while(tmp){
			if(equ_val(tmp->val,s)){
				return tmp;
			}
			tmp=tmp->next;
		}
		return NULL;
	}
	public:
	//if used outside  must lock and unlock
	T* find_val(S s){
		queue_entry<T> *tmp=find_entry(s);
		if(tmp==NULL){
			unlock();
			return NULL;
		}
		return tmp->val;
	}

	//find is always dangers, since the entry could be freed
	//right after the find
	
	T* unqueue_search(S s){
		lock();
		queue_entry<T> *tmp=find_entry(s);
		if(tmp==NULL){
			unlock();
			return NULL;
		}
		count--;
		T *val=tmp->val;
		if(tmp->prev)
			tmp->prev->next=tmp->next;
		else
			head=tmp->next;
		if(tmp->next){
			tmp->next->prev=tmp->prev;
		}else
			tail=tmp->prev;
		unlock();
		delete tmp;
		return val;
	}

	T* unqueue_entry_search(queue_entry<T>* qs){
		lock();
		queue_entry<T> *tmp=head;
		while(tmp){
			if(tmp==qs){
				break;
			}
			tmp=tmp->next;
		}
		if(tmp==NULL){
			unlock();
			return NULL;
		}
		count--;
		T *val=tmp->val;
		if(tmp->prev)
			tmp->prev->next=tmp->next;
		else
			head=tmp->next;
		if(tmp->next)
			tmp->next->prev=tmp->prev;
		else
			tail=tmp->prev;
		unlock();
		delete tmp;
		return val;
	}
};

/*
for char * class
class gen_char_queue: public gen_winlock_queue<char,char*>{
	public:
		int equ_val(char *r,char*p){
			if(strcmp(r,p)==0)return 1;
			return 0;
		}
		//void Remove_val(char*val){
		//	delete val;
		//}
};
*/

////////////////////// end of this header file ////////////////
#endif

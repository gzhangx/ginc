//////////////////////////////////////////////////////////////
//
//File name:	cgthread.h
//Author:		Gang Zhang (gzhang@icx.net)
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//Date:			06/27/1998
//Version:		1.0
//Note:			Need to include cgthread.cpp (with the static
//				true thread function).
//				Also, need to implement
//				virtual void before_die()=0;
//				virtual int InitInstance(void)=0;
//				InitInstance is the one need to put run loop in
//				before_die will be called before the exist of the
//				thread, put needed init, free or unblock there
//				before_die is implemented so a delete of the thread
//				is the only thing needed to stop and free a thread
//				Put KillThread to virtual and implement a new one
//				is useless.
//
//Important:			KillThread is not thread safe.
//				Don't delete it in one thread and
//				do killThread in another thread.
//Revision 1.0:			Must call KillThread in destructor,
//				Other wise if a thread allocated (new)
//				Created (CreateThread() ) and then
//				immidtaly deleted, The pure virtual
//				InitInstance() might be called instead of
//				the one intended.
//
//Revision 1.1:		KillThread() now accept a parameter of
//				Kill timeout.  It returns TRUE if thread ends
//				In the given time or FALSE if thread is not dead yet
//				If no parameters are given, It wait on the thread 
//				for ever
//
//Revision 1.2:		Aug 28,1998 GZ:	
//			#define GANG_DLL_EXPORT __declspec( dllexport )
//			and the class will be exportable to a dll
//////////////////////////////////////////////////////////////

#include <windows.h>
#include <process.h>
/* Example usage
class USEfullThread :public CGThread{
	~USEfullThread(){
		KillThread();	//This call is needed.  when
				//you create this class, create thread
				//and delete it right after, since InitInstance
				//is a pure Virtual, ::CreateThread() will fail
	}
	public:
	int InitInstance(void){
		while(is_alive()){
			//
			//Do something
			//
		}
		return 0;
	}
};

USEfullThread u=new USEfullThread();
u->CreateThread();
//begin to kill.  If have a lot of threads,
//let them go die first then delete them
u->go_exit();
delete u;
*/

#ifndef CGTHREAD_HEADERFILE
#define CGTHREAD_HEADERFILE

#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif


//GANG_DLL_EXPORT unsigned int __stdcall Global_Gang_Thread_Func( void * class_name );

class GANG_DLL_EXPORT gCThread{
protected:
	BOOL live_sign;
	HANDLE the_thread;
public:
	gCThread(){
		live_sign=TRUE;
		the_thread=NULL;
	}
	virtual gCThread::~gCThread();
	HANDLE gCThread::CreateThread(unsigned state=0);

	inline BOOL is_alive(){
		return live_sign;
	}

	inline void go_exit(){
		live_sign=FALSE;
	}

	DWORD gCThread::ResumeThread();
	DWORD gCThread::SuspendThread( );

	//use saved_thread keep the thread handle
	//and set the_thread to NULL.  so 
	//main program (InitInstance) should check 
	//for is_alive() to see if I should quit
	virtual BOOL gCThread::KillThread(DWORD wms=INFINITE);

protected:
	virtual BOOL InitInstance(void)=0;
	static unsigned int __stdcall Global_Gang_Thread_Func( void * class_name );
};


#endif


/*
class ctls : public gCThread{
public:
	int ident;
	ctls(int i){
		ident=i;
	}
	int InitInstance(){
		int i=0;
		while(is_alive()){
			//printf(" %i it is %i\n",ident,i++);
			Sleep(10);
		}
		return 0;
	}
};

#define MAX 100
int cmain(void){
	ctls *t[MAX];
	for(int i=0;i<MAX;i++){
		t[i]=new ctls(i);
		if(t[i]){
			t[i]->CreateThread();
		}else
			printf("Can't create %i\n",i);
	}

	
	Sleep(100);
	for(i=0;i<MAX;i++){
		if(t[i]){
			delete t[i];
		}
	}
	printf("Done\n");
	return 0;
}


*/


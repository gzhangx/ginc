//////////////////////////////////////////////////////////////////////////
//
//	Program:	gcthread.cpp
//	DESC:		implement thread function
//	Author:		Gang Zhang
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Date:		06/26/1998
//	Revisions:
//////////////////////////////////////////////////////////////////////////

#include "gerror.h"
#include "gcthread.h"

#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif

unsigned int __stdcall gCThread::Global_Gang_Thread_Func(void * class_name)
{
	gCThread * ptd=static_cast<gCThread*>(class_name);
	//return ptd->InitInstance();
	_endthreadex(ptd->InitInstance());
	return 0;	//useless
}


	//virutal destrostur
	gCThread::~gCThread(){
		KillThread();
	}

	//default state=0
	HANDLE gCThread::CreateThread(unsigned state){	//CREATE_SUSPENDED
		unsigned int ident;
		if(the_thread!=NULL){
			throw gerror_str("gCThread: try to create thread again while thread is alive",ERR_THREAD_CREATE_AGAIN);
		}
		live_sign=TRUE;
		the_thread= (HANDLE)_beginthreadex(NULL,0,Global_Gang_Thread_Func,
			(void*)this,state,&ident);
		if(the_thread==(HANDLE)-1){
			live_sign=FALSE;
			the_thread=NULL;
			throw gerror_str("gCThread: create thread failed",ERR_THREAD_CREATE);
		}
		return the_thread;
	}

	DWORD gCThread::ResumeThread(){
		return ::ResumeThread(the_thread);
	}
	DWORD gCThread::SuspendThread( ){
		return ::SuspendThread(the_thread);
	}

	//this is a virtual function
	//use saved_thread keep the thread handle
	//and set the_thread to NULL.  so 
	//main program (InitInstance) should check 
	//for is_alive() to see if I should quit
	//wms default =INFINITE
	BOOL gCThread::KillThread(DWORD wms){
		if(the_thread){
			go_exit();
			ResumeThread();
			if(WaitForSingleObject(the_thread,wms)==WAIT_TIMEOUT){
				return FALSE;
			}
			CloseHandle(the_thread);
			the_thread=NULL;
		}

		return TRUE;
	}



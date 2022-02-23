#ifndef GANGGETPROCESSMEMHEADERFILE
#define GANGGETPROCESSMEMHEADERFILE
#include <windows.h>
#include <tlhelp32.h> // needed for tool help declarations


// Type definitions for pointers to call tool help functions. 
typedef BOOL (WINAPI *MODULEWALK)(HANDLE hSnapshot, 
    LPMODULEENTRY32 lpme);
typedef BOOL (WINAPI *THREADWALK)(HANDLE hSnapshot, 
    LPTHREADENTRY32 lpte); 
typedef BOOL (WINAPI *PROCESSWALK)(HANDLE hSnapshot,
    LPPROCESSENTRY32 lppe); 
typedef HANDLE (WINAPI *CREATESNAPSHOT)(DWORD dwFlags, 
    DWORD th32ProcessID); 
 
// File scope globals. These pointers are declared because of the need 

// to dynamically link to the functions.  They are exported only by 
// the Windows 95 kernel. Explicitly linking to them will make this
// application unloadable in Microsoft(R) Windows NT(TM) and will 
// produce an ugly system dialog box. 

//class throws char *
class CGProcessMem{
private :
  HANDLE mProc;
	MODULEENTRY32 mMe32;
  static CREATESNAPSHOT pCreateToolhelp32Snapshot; 
  static MODULEWALK  pModule32First  ; 
  static MODULEWALK  pModule32Next   ; 
  static PROCESSWALK pProcess32First ; 
  static PROCESSWALK pProcess32Next  ; 

  static THREADWALK  pThread32First  ; 
  static THREADWALK  pThread32Next   ; 

  CGProcessMem(HANDLE proc,MODULEENTRY32 me32);
  void close();
public:
  ~CGProcessMem();

  //DWORD   modBaseSize;
  //MODULEENTRY32 *getModule(){return &mMe32;}
  unsigned long getModuleSize(){return mMe32.modBaseSize;}
  static BOOL InitToolhelp32 (void) ;


  
  
  static CGProcessMem* getProcessByName(char *name);

  BOOL readMem(char *buf,int at,int size,unsigned long *sizeread);
  BOOL writeMem(char *buf,int at,int size,unsigned long *sizewrite);
  
  //static BOOL GetProcessList (void (callback)(MODULEENTRY32*));
  

  protected:
  // Returns TRUE if there is information about the specified module or
  //   FALSE if it could not enumerate the modules in the process or 
  //   the module is not found in the process. 
  // dwPID - identifier of the process that owns the module to 
  //   retrieve information about. 
  // dwModuleID - tool help identifier of the module within the 
  //   process
  // lpMe32 - structure to return data about the module 
  // cbMe32 - size of the buffer pointed to by lpMe32 (to ensure

  //   the buffer is not over filled) 
  static BOOL GetProcessModule (DWORD dwPID, 
    //02/10/2002  changed to use mod name to id module
    char *modName,
    //DWORD dwModuleID, 
        LPMODULEENTRY32 lpMe32, DWORD cbMe32) ;

};


#endif

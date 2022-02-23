/////////////////////////////////////////////////////////////////////////////
//
//	File:		unreg_guid.h
//	Author:		Gang Zhang (gzhang@icx.net)
//	E-mail:		gzhang_ocx@yahoo.com or gazhang@geocities.com
//	HTTP:		http://members.xoom.com/gzhang
//	Date:		10/20/1998
//	Description:	Unregister guid class
//			UnReg_GUID(guid,"i");
//			i for interface, c for clsid, t for typelib			
//
//	Revisions:	
//
/////////////////////////////////////////////////////////////////////////////

#include <mapiguid.h>
#include <iostream.h>
#include <strstrea.h>
#include <creg.h>


#ifndef GANG_UNREG_GUID_HEADER_FILE
#define GANG_UNREG_GUID_HEADER_FILE

#ifndef GANG_DLL_EXPORT
#define GANG_DLL_EXPORT
#endif


class GANG_DLL_EXPORT UnReg_GUID{
public:
	enum {
		STRUID_SIZE=1+8+1+4+1+4+1+4+1+12+1+1
	};

	UnReg_GUID(const GUID &ui,const char *type,HKEY thehkey=HKEY_CLASSES_ROOT){
		char id[STRUID_SIZE];
		ostrstream osm(id,STRUID_SIZE);
		osm<<"{"<<hex;
		osm.width(8);
		osm.fill('0');
		osm.setf(ostrstream::right|ostrstream::uppercase);
		osm<<ui.Data1<<"-";
		osm.width(4);
		osm<<ui.Data2<<"-";
		osm.width(4);
		osm<<ui.Data3<<"-";
		for(int i=0;i<2;i++){
			osm.width(2);
			osm<<(int)(unsigned char)ui.Data4[i];
		}
		osm<<"-";
		for(;i<8;i++){
			osm.width(2);
			osm<<(int)(unsigned char)ui.Data4[i];
		}
		osm<<"}"<<ends;
		GCRegKey rkey(thehkey);
		char *subkey=NULL;
		switch(*type){
			case 'C':
			case 'c':
				subkey="CLSID";
				break;
			case 'I':
			case 'i':
				subkey="Interface";
				break;
			case 'T':
			case 't':
			case 'L':
			case 'l':
				subkey="Typelib";
				break;
		}
		if(subkey==NULL){
			throw gerror_str("Subkey need to be CLSID, Interface or Typelib");
		}
		GCRegKey key(&rkey,subkey);
		key.DeleteSubKey(id);
	}
};


#endif
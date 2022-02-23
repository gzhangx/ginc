////////////////////////////////////////////////////////////////////////
//
//	FILE:	gCSqlConnection.h
//	Author:	Gang Zhang
//	E-mail: gzhang_ocx@yahoo.com, gazhang@geocities.com
//	HTTP:	http://members.xoom.com/gzhang
//	Date:	03/22/1999	
//
//	Description: implement C++ wrapper for general sql functions
//	


// gCSqlConnection.h: interface for the gCSqlConnection class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GCSQLCONNECTION_H__7F1D2FD2_E069_11D2_AE03_0004AC6001CC__INCLUDED_)
#define AFX_GCSQLCONNECTION_H__7F1D2FD2_E069_11D2_AE03_0004AC6001CC__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <gerror.h>
#include <sql.h>
#include <sqlext.h>

class gCSqlConnection;
class gCSqlStatement;
class GANG_DLL_EXPORT gCSql_error_str:public gerror_str{
protected:
	virtual void init(){
		set_class("gSql_error_str");
	}
public:
	static RETCODE get_sql_err(gCSqlStatement *con,UCHAR *state,SDWORD *err,UCHAR*errMsg,int szerrMsgmax,SWORD *szerr);
	gCSql_error_str(const char *str1=NULL,const GERROR_CODES err_num=ERR_OK):gerror_str(str1,err_num){
		init();
	}
	gCSql_error_str(const gerror_str &err):gerror_str(err){
		init();
	}
	static RETCODE get_sql_err(gCSqlConnection *con,UCHAR *state,SDWORD *err,UCHAR*errMsg,int szerrMsgmax,SWORD *szerr);
};



class gCSqlConnection  
{
protected:
	HENV    m_henv;
	HDBC    m_hdbc;
	RETCODE m_retcode;

public:
	//void RollBack();
	//void Commit();
	HENV get_env() const {return m_henv;}
	HDBC get_dbc() const {return m_hdbc;}

	operator HDBC() const {return get_dbc();}
	void de_init();
	void sql_init(int timeout=5);	//5s timeout
	gCSqlConnection(int timeout=5);
	virtual ~gCSqlConnection();
	void Commit();
	void RollBack();
};

enum{
	GSQL_INFO_MAX=512
};
class gCSqlColInfo{
public:
	UCHAR m_szColName[GSQL_INFO_MAX];
	SWORD m_cbColNameMax;
	SWORD m_pcbColName;
	SWORD m_pfSqlType;
	UDWORD m_pcbColDef;
	SWORD m_pibScale;
	SWORD m_pfNullable;
	gCSqlColInfo(){
		m_szColName[0]=0;
		m_cbColNameMax=GSQL_INFO_MAX;
		m_pcbColName=0;
	}
};
class gCSqlStatement{
protected:
	gCSqlConnection *m_cdbc;
	HSTMT   m_hstmt;
	RETCODE m_retcode;
	BOOL m_connected;
public:
	RETCODE DescribeCol(UWORD icol,gCSqlColInfo *info);
	RETCODE ResCols(SWORD *cols);
	RETCODE BindCol(UWORD irow,SWORD type,PTR val,SDWORD sizemax,SDWORD *rsize=NULL);
	RETCODE SetPos(UWORD irow);
	RETCODE Prepare(const char *strprep);
	RETCODE MoveTo(UDWORD *crow,UWORD *res);
	RETCODE MoveLast(UDWORD *crow,UWORD *res);
	RETCODE MoveFirst(UDWORD *crow,UWORD *res);
	RETCODE MovePrev(UDWORD *crow,UWORD *res);
	RETCODE MoveNext(UDWORD *rows,UWORD *res=NULL);
	RETCODE Fetch();
	RETCODE ExecDirect(const char *str);
	BOOL eval_retcode();	//returns SQL_SUCCESS for no error
	RETCODE Execute();
	HDBC get_dbc() const {return m_cdbc->get_dbc();}
	HENV get_env() const {return m_cdbc->get_env();}
	HSTMT get_stmt() const {return m_hstmt;}

	virtual ~gCSqlStatement();
	void de_init();
	void sql_connect(gCSqlConnection *cdbc,unsigned char *dsn,unsigned char *usr=NULL,unsigned char *pass=NULL);
	gCSqlStatement();
};

#endif // !defined(AFX_GCSQLCONNECTION_H__7F1D2FD2_E069_11D2_AE03_0004AC6001CC__INCLUDED_)

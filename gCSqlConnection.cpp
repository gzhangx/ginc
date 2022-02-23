////////////////////////////////////////////////////////////////////////
//
//	FILE:	gCSqlConnection.cpp
//	Author:	Gang Zhang
//	E-mail: gzhang_ocx@yahoo.com, gazhang@geocities.com
//	HTTP:	http://members.xoom.com/gzhang
//	Date:	03/22/1999
//	Description: implement C++ wrapper for general sql functions
//	

// gCSqlConnection.cpp: implementation of the gCSqlConnection class.
//
//////////////////////////////////////////////////////////////////////


#include "gCSqlConnection.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

RETCODE gCSql_error_str::get_sql_err(gCSqlConnection *con,UCHAR *state,SDWORD *err,UCHAR*errMsg,int szerrMsgmax,SWORD *szerr){
		return SQLError(con->get_env(), con->get_dbc(), SQL_NULL_HSTMT,
			state, err, errMsg, szerrMsgmax,
			szerr);
}

RETCODE gCSql_error_str::get_sql_err(gCSqlStatement * con, UCHAR * state, SDWORD * err, UCHAR * errMsg, int szerrMsgmax, SWORD * szerr)
{
	return SQLError(con->get_env(), con->get_dbc(), con->get_stmt(),
			state, err, errMsg, szerrMsgmax,
			szerr);
}

////////////////////////////////////////////////////////////////////////
//
//	Implement of sql connection
//
gCSqlConnection::gCSqlConnection(int timeout)
{
	m_henv=NULL;
	m_hdbc=NULL;
	m_retcode=0;
	sql_init(timeout);
}

gCSqlConnection::~gCSqlConnection()
{
	de_init();
}

void gCSqlConnection::sql_init(int timeout)
{
	de_init();
	m_retcode = SQLAllocEnv(&m_henv);              // Environment handle
	if (m_retcode != SQL_SUCCESS)
		throw gCSql_error_str("SQL: Can't allocate enviroment",ERR_SQL_ALLOC_ENV);
	m_retcode = SQLAllocConnect(m_henv, &m_hdbc); /* Connection handle */
    if (m_retcode != SQL_SUCCESS)
		throw gCSql_error_str("SQL: Can't allocate connection",ERR_SQL_ALLOC_CONNECTION);

	// Set login timeout to timeout seconds.
	SQLSetConnectOption(m_hdbc, SQL_LOGIN_TIMEOUT, timeout);
}

void gCSqlConnection::de_init()
{
	if(m_hdbc)SQLFreeConnect(m_hdbc);
	m_hdbc=NULL;
	if(m_henv)SQLFreeEnv(m_henv);
	m_henv=NULL;
}


void gCSqlConnection::Commit()
{
	m_retcode=SQLTransact(get_env(), get_dbc(), SQL_COMMIT);
	switch(m_retcode){
	case SQL_ERROR:
		throw gCSql_error_str("SQL: Commit error",ERR_SQL_COMMIT);
	case SQL_INVALID_HANDLE:
		throw gCSql_error_str("SQL: Commit error",ERR_SQL_INVALID_HANDLE);
	}
}

void gCSqlConnection::RollBack()
{
	m_retcode=SQLTransact(get_env(), get_dbc(), SQL_ROLLBACK);
	switch(m_retcode){
	case SQL_ERROR:
		throw gCSql_error_str("SQL: Commit error",ERR_SQL_COMMIT);
	case SQL_INVALID_HANDLE:
		throw gCSql_error_str("SQL: Commit error",ERR_SQL_INVALID_HANDLE);
	}
}


////////////////////////////////////////////////////////////////////////
//
//	Implement of sql statement
//
gCSqlStatement::gCSqlStatement()
{
	m_cdbc=NULL;
	m_hstmt=NULL;
	m_retcode=0;
	m_connected=FALSE;
}

void gCSqlStatement::sql_connect(gCSqlConnection* cdbc,unsigned char *dsn,unsigned char *usr,unsigned char *pass)
{
	// Connect to data source
	m_cdbc=cdbc;
	m_retcode = SQLConnect(*m_cdbc, dsn, SQL_NTS, usr, SQL_NTS,pass, SQL_NTS);
	if (eval_retcode()== SQL_SUCCESS){
		// Process data after successful connection
		m_retcode = SQLAllocStmt(*m_cdbc, &m_hstmt); // Statement handle
		if (m_retcode != SQL_SUCCESS) {
			SQLDisconnect(*m_cdbc);
			m_cdbc=NULL;
			//SQLFreeStmt(m_hstmt, SQL_DROP);
			throw gCSql_error_str("SQL: Can't allocate statement",ERR_SQL_ALLOC_STATEMENT);
		}
	}else{
		throw gCSql_error_str("SQL: Can't connect to dsn ",ERR_SQL_DSN_CONNECT);
	}
}

void gCSqlStatement::de_init()
{
	if(m_hstmt)SQLFreeStmt(m_hstmt, SQL_DROP);
	m_hstmt=NULL;
	if(m_cdbc)SQLDisconnect(*m_cdbc);
	m_cdbc=NULL;
}

gCSqlStatement::~gCSqlStatement()
{
	de_init();
}

RETCODE gCSqlStatement::Execute()
{
	m_retcode = SQLExecute(m_hstmt);
	eval_retcode();
	return m_retcode;
}

BOOL gCSqlStatement::eval_retcode()
{
	switch(m_retcode){
	case SQL_SUCCESS:
	case SQL_SUCCESS_WITH_INFO:
		return SQL_SUCCESS;
	case SQL_NO_DATA_FOUND:
		throw gCSql_error_str("SQL: No Data Found",ERR_SQL_NO_DATA_FOUND);
	case SQL_STILL_EXECUTING:
		throw gCSql_error_str("SQL: Still executing",ERR_SQL_STILL_EXECUTING);
	case SQL_ERROR:
		throw gCSql_error_str("SQL: SQL_ERROR",ERR_SQL_ERR);
	case SQL_INVALID_HANDLE:
		throw gCSql_error_str("SQL: Invalid Handle",ERR_SQL_INVALID_HANDLE);
	default:
		throw gCSql_error_str("SQL: error not impelemented error",ERR_SQL_ERR);
		return m_retcode;
	}
}

RETCODE gCSqlStatement::ExecDirect(const char * str)
{
	m_retcode=SQLExecDirect(m_hstmt,(UCHAR*)str,SQL_NTS);
	eval_retcode();
	return m_retcode;
}

RETCODE gCSqlStatement::Fetch()
{
	m_retcode=SQLFetch(m_hstmt);
	eval_retcode();
	return m_retcode;
}

RETCODE gCSqlStatement::MoveNext(UDWORD * crow, UWORD * res)
{
	UWORD tres[1];
	if(res==NULL){
		res=tres;
	}
	m_retcode = SQLExtendedFetch(m_hstmt, SQL_FETCH_NEXT, 1, crow, res);
	eval_retcode();
	return m_retcode;
}



RETCODE gCSqlStatement::MovePrev(UDWORD * crow, UWORD * res)
{
	UWORD tres[1];
	if(res==NULL){
		res=tres;
	}
	m_retcode = SQLExtendedFetch(m_hstmt, SQL_FETCH_PRIOR, 1, crow, res);
	eval_retcode();
	return m_retcode;
}

RETCODE gCSqlStatement::MoveFirst(UDWORD * crow, UWORD * res)
{
	UWORD tres[1];
	if(res==NULL){
		res=tres;
	}
	m_retcode = SQLExtendedFetch(m_hstmt, SQL_FETCH_FIRST, 1, crow, res);
	eval_retcode();
	return m_retcode;
}

RETCODE gCSqlStatement::MoveLast(UDWORD * crow, UWORD * res)
{
	UWORD tres[1];
	if(res==NULL){
		res=tres;
	}
	m_retcode = SQLExtendedFetch(m_hstmt, SQL_FETCH_LAST, 1, crow, res);
	eval_retcode();
	return m_retcode;
}

RETCODE gCSqlStatement::MoveTo(UDWORD * crow, UWORD * res)
{
	UWORD tres[1];
	if(res==NULL){
		res=tres;
	}
	m_retcode = SQLExtendedFetch(m_hstmt, SQL_FETCH_ABSOLUTE, 1, crow, res);
	eval_retcode();
	return m_retcode;
}

RETCODE gCSqlStatement::Prepare(const char * strprep)
{
	m_retcode=SQLPrepare(m_hstmt, (UCHAR*)strprep, SQL_NTS);
	eval_retcode();
	return m_retcode;
}

RETCODE gCSqlStatement::SetPos(UWORD irow)
{
	m_retcode=SQLSetPos(m_hstmt, irow, SQL_POSITION, SQL_LOCK_NO_CHANGE);
	eval_retcode();
	return m_retcode;
}

RETCODE gCSqlStatement::BindCol(UWORD irow,SWORD type, PTR val, SDWORD sizemax, SDWORD * rsize)
{
	SDWORD tsize;
	if(rsize==NULL)rsize=&tsize;
	m_retcode=SQLBindCol(m_hstmt,irow,type,val,sizemax,rsize);
	eval_retcode();
	return m_retcode;
}

RETCODE gCSqlStatement::ResCols(SWORD * cols)
{
	m_retcode=SQLNumResultCols(m_hstmt, cols);
	eval_retcode();
	return m_retcode;
}


RETCODE gCSqlStatement::DescribeCol(UWORD icol,gCSqlColInfo * info)
{
	m_retcode=SQLDescribeCol(m_hstmt,
		icol, 
		info->m_szColName,
		info->m_cbColNameMax,
		&info->m_pcbColName,
		&info->m_pfSqlType,
		&info->m_pcbColDef,
		&info->m_pibScale,
		&info->m_pfNullable);
	eval_retcode();
	return m_retcode;
}

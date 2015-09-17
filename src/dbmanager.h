/*****************************************
> File Name : dbmanager.h
> Description : dbmanager.h  file
> Author : linden
> Date : 2015-09-14
*******************************************/

#ifndef _DBMANAGER_H_
#define _DBMANAGER_H_

#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <list>
#include <stdio.h>

using namespace std;

class CDBManager
{  
public:  
    CDBManager();  
    ~CDBManager();
		
    bool	initDB(string host, string user, string pwd, string db_name);  
    bool	executeSQL(string sql);
	bool	user_register_func(string user_login, string user_pass);
	bool	user_login_func(string user_login, string user_pass);
private:  
    MYSQL *connection;  
    MYSQL_RES *result;  
    MYSQL_ROW row;  
};  

#endif



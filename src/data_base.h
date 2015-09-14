/*****************************************
> File Name : data_base.h
> Description : data_base.h  file
> Author : linden
> Date : 2015-09-14
*******************************************/

#ifndef _DATABASE_H
#define _DATABASE_H

#include <iostream>  
#include <string>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <list>
#include <stdio.h>

using namespace std;

class CDatabase
{  
public:  
    CDatabase();  
    ~CDatabase();
		
    bool	initDB(string host, string user, string pwd, string db_name);  
    bool	executeSQL(string sql);
	bool	user_register(string user_login, string user_pass);
private:  
    MYSQL *connection;  
    MYSQL_RES *result;  
    MYSQL_ROW row;  
};  

#endif



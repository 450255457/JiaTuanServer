/*****************************************
> File Name : data_base.c
> Description : data_base.c  file
> Author : linden
> Date : 2015-09-14
*******************************************/

#include "data_base.h"

CDatabase::CDatabase()  
{  
    connection = mysql_init(NULL); // 初始化数据库连接变量  
    if(connection == NULL)  
    {  
        cout << "Error:" << mysql_error(connection);  
        exit(1);
    }  
}  
  
CDatabase::~CDatabase()  
{  
    if(connection != NULL)  // 关闭数据库连接  
    {  
        mysql_close(connection);  
    }  
}  
  
bool CDatabase::initDB(string host, string user, string pwd, string db_name)  
{  
    // 函数mysql_real_connect建立一个数据库连接  
    // 成功返回MYSQL*连接句柄，失败返回NULL  
    connection = mysql_real_connect(connection, host.c_str(),  
            user.c_str(), pwd.c_str(), db_name.c_str(), 0, NULL, 0);  
    if(connection == NULL)  
    {  
        cout << "Error:" << mysql_error(connection);  
        exit(1);  
    }  
    return true;  
}  
  
bool CDatabase::executeSQL(string sql)  
{  
    // mysql_query()执行成功返回0，失败返回非0值。与PHP中不一样  
    if(mysql_query(connection, sql.c_str()))
    {  
        cout << "Query Error:" << mysql_error(connection);  
        exit(1);  
    }  
    else  
    {  
        result = mysql_use_result(connection); // 获取结果集  
        // mysql_field_count()返回connection查询的列数  
        for(int i=0; i < mysql_field_count(connection); ++i)  
        {  
            // 获取下一行  
            row = mysql_fetch_row(result);  
            if(row <= 0)  
            {  
                break;  
            }  
            // mysql_num_fields()返回结果集中的字段数  
            for(int j=0; j < mysql_num_fields(result); ++j)  
            {  
                cout << row[j] << " ";  
            }  
            cout << endl;  
        }  
        // 释放结果集的内存  
        mysql_free_result(result);  
    }  
    return true;  
}


/************************************************
Function : user_register
Description : 用户注册
Input : string user_login
		string user_pass
Output :
Return : true/false
Others :
*************************************************/
bool CDatabase::user_register(string user_login, string user_pass)
{	
	//string user_register_sql = "insert into jt_users (user_login,user_pass,user_registered) values ("test","test",now());";
	string user_register_sql = "insert into jt_users (user_login,user_pass,user_registered) values (" + user_login + "," + user_pass + ",now())";
    // mysql_query()执行成功返回0，失败返回非0值
	if (mysql_query(connection, user_register_sql.c_str()))
    {  
        cout << "Query Error:" << mysql_error(connection);
        return false;  
    }
    return true;
}

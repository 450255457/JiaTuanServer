/*****************************************
> File Name : data_base.c
> Description : data_base.c  file
> Author : linden
> Date : 2015-09-14
*******************************************/

#include "data_base.h"

CDatabase::CDatabase()  
{  
    connection = mysql_init(NULL); // ��ʼ�����ݿ����ӱ���  
    if(connection == NULL)  
    {  
        cout << "Error:" << mysql_error(connection);  
        exit(1);
    }  
}  
  
CDatabase::~CDatabase()  
{  
    if(connection != NULL)  // �ر����ݿ�����  
    {  
        mysql_close(connection);  
    }  
}  
  
bool CDatabase::initDB(string host, string user, string pwd, string db_name)  
{  
    // ����mysql_real_connect����һ�����ݿ�����  
    // �ɹ�����MYSQL*���Ӿ����ʧ�ܷ���NULL  
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
    // mysql_query()ִ�гɹ�����0��ʧ�ܷ��ط�0ֵ����PHP�в�һ��  
    if(mysql_query(connection, sql.c_str()))
    {  
        cout << "Query Error:" << mysql_error(connection);  
        exit(1);  
    }  
    else  
    {  
        result = mysql_use_result(connection); // ��ȡ�����  
        // mysql_field_count()����connection��ѯ������  
        for(int i=0; i < mysql_field_count(connection); ++i)  
        {  
            // ��ȡ��һ��  
            row = mysql_fetch_row(result);  
            if(row <= 0)  
            {  
                break;  
            }  
            // mysql_num_fields()���ؽ�����е��ֶ���  
            for(int j=0; j < mysql_num_fields(result); ++j)  
            {  
                cout << row[j] << " ";  
            }  
            cout << endl;  
        }  
        // �ͷŽ�������ڴ�  
        mysql_free_result(result);  
    }  
    return true;  
}


/************************************************
Function : user_register
Description : �û�ע��
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
    // mysql_query()ִ�гɹ�����0��ʧ�ܷ��ط�0ֵ
    if(mysql_query(connection, sql.c_str()))  
    {  
        cout << "Query Error:" << mysql_error(connection);  
        return false;  
    }
    return true;
}

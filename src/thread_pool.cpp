/*****************************************
> File Name		: thread_pool.cpp
> Description	: �̳߳�
> Author		: linden
> Date			: 2015-08-07
*******************************************/

#include "thread_pool.h"
#include "server.h"

CServer CS;

int Erase_APP(char *Buff,unsigned int nFileLen);

threadpool::threadpool(){
	
}

threadpool::~threadpool(){
	
}

/*******************************************************************************
Function	: threadpool_init
Description : �̳߳صĳ�ʼ��
Input		: int thread_number,�̳߳����̵߳�����
			  int max_requests,�����������������,�ȴ���������������
Output		:
Return		: 0
Others		:
*******************************************************************************/
int threadpool::threadpool_init(int thread_number, int max_requests){
	m_stop = false;
	m_threads = NULL;
	m_max_requests = max_requests;
    if( ( thread_number <= 0 ) || ( max_requests <= 0 ) ){
        throw std::exception();
    }

    m_threads = new pthread_t[ thread_number ];			//��̬���ɶ�������
    if( ! m_threads )
    {
        throw std::exception();
    }
	//����thread_number���߳�,�������Ƕ�����Ϊ�����߳�
    for ( int i = 0; i < thread_number; ++i )
    {
        //printf( "create the %dth thread\n", i );
        if( pthread_create( m_threads + i, NULL, worker,this ) != 0 )
        {
            delete [] m_threads;
            throw std::exception();
        }
        if( pthread_detach( m_threads[i] ) )
        {
            delete [] m_threads;
            throw std::exception();
        }
    }
	return 0;
}

void threadpool::threadpool_destroy(){
	delete [] m_threads;
    m_stop = true;
}

/*******************************************************************************
Function	: append
Description : �̳߳���������е����
Input		: void* request,�������Ĳ���ָ��
Output		:
Return		: true/false
Others		:
*******************************************************************************/
bool threadpool::append( void* request )
{
	//������������ʱһ��Ҫ����,��Ϊ���������̹߳���
    m_queuelocker.lock();
    if ( m_workqueue.size() > m_max_requests )
    {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back( request );
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

//�����߳����еĺ���,�����ϴӹ���������ȡ������ִ��֮
void* threadpool::worker( void* arg )
{
	threadpool* pool = ( threadpool* )arg;
    pool->run();
    return pool;
}

void threadpool::run()
{
	CPublicFunction MyPF;
	//printf("pthread_self = %x\n",pthread_self());
    while ( ! m_stop )
    {
        m_queuestat.wait();
        m_queuelocker.lock();
        if ( m_workqueue.empty() )
        {
            m_queuelocker.unlock();
            continue;
        }
        void* request = m_workqueue.front();	//���ص�һ��Ԫ�ص�����
        m_workqueue.pop_front();				//ɾ������ͷ��һԪ��
        m_queuelocker.unlock();
        if ( ! request )
        {
            continue;
        }
        //request->process();
		thread_para *thread_arg = (thread_para *)request;
		int sockfd = thread_arg->sockfd;
		int epollfd = thread_arg->epollfd;
		//printf( "start new thread to receive data on sockfd: %d\n", sockfd );
		char recv_buf[BUFMAXSIZE];  
		memset( recv_buf, '\0', BUFMAXSIZE );
		//ѭ����ȡsocket�ϵ�����,ֱ������EAGAIN����.
		while( 1 ){
			int ret = recv( sockfd, recv_buf, BUFMAXSIZE - 1, 0 ); 
			if( ret == 0 ){  
				close( sockfd );   
				//printf( "foreiner closed the connection\n" );   
				break;  
			}   
			else if( ret < 0 ){
				//���ڷ�����IO,���������������ʾ�����Ѿ�ȫ����ȡ��ϡ��˺�epoll�����ٴδ���sockfd�ϵ�EPOLLIN�¼�����������һ�ζ�����
				if((errno == EAGAIN) || (errno == EWOULDBLOCK)){
					CS.reset_oneshot( epollfd, sockfd );
					//printf( "read later\n" );
					break;            
				}        
			}
			printf("recv_buf = %s\n", recv_buf);
		}
		
		//printf("Test:sockfd = %d,epoll = %d\n",sockfd,epollfd);
		//printf("pthread_self = %x\n",pthread_self());
    }
	return;
}

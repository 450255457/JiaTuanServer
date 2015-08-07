/*****************************************
> File Name		: thread_pool.cpp
> Description	: 线程池
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
Description : 线程池的初始化
Input		: int thread_number,线程池中线程的数量
			  int max_requests,请求队列中最多允许的,等待处理的请求的数量
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

    m_threads = new pthread_t[ thread_number ];			//动态生成对象数组
    if( ! m_threads )
    {
        throw std::exception();
    }
	//创建thread_number个线程,并将它们都设置为脱离线程
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
Description : 线程池中任务队列的添加
Input		: void* request,传进来的参数指针
Output		:
Return		: true/false
Others		:
*******************************************************************************/
bool threadpool::append( void* request )
{
	//操作工作队列时一定要加锁,因为它被所有线程共享
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

//工作线程运行的函数,它不断从工作队列中取出任务并执行之
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
        void* request = m_workqueue.front();	//返回第一个元素的引用
        m_workqueue.pop_front();				//删除链表头的一元素
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
		//循环读取socket上的数据,直到遇到EAGAIN错误.
		while( 1 ){
			int ret = recv( sockfd, recv_buf, BUFMAXSIZE - 1, 0 ); 
			if( ret == 0 ){  
				close( sockfd );   
				//printf( "foreiner closed the connection\n" );   
				break;  
			}   
			else if( ret < 0 ){
				//对于非阻塞IO,下面的条件成立表示数据已经全部读取完毕。此后，epoll就能再次触发sockfd上的EPOLLIN事件，以驱动下一次读操作
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

#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
using namespace std;

template<class T>
class block_queue 
{
public:

    block_queue(int max_size = 1000);
    ~block_queue();
    
    //清空队列
    void clear();
    //判断队列是否满了    
    bool full();
    //判断队列是否为空
    bool empty();
    //返回队首元素
    bool front(T &value);
    //返回队尾元素
    bool back(T &value);
    //返回当前队列长度
    int size();
    //返回最大尺寸
    int max_size();
    //添加
    bool push(const T &item);
    //减少
    bool pop(T &item);

private:
    
    pthread_mutex_t lock;
    pthread_cond_t  cond;
    T *m_array;
    int m_size;
    int m_max_size;
    int m_front;
    int m_back;
};




template<class T>
block_queue<T>::block_queue(int max_size)
    :m_max_size(max_size),m_size(0),m_front(-1),m_back(-1)
{
    if(max_size <= 0)
        exit(-1);
    m_array = new T[max_size];
    
    if(pthread_mutex_init(&lock,NULL) != 0)
        throw exception();   //抛出异常

    if(pthread_cond_init(&cond,NULL) != 0)
        throw exception();

}
template<class T>
block_queue<T>::~block_queue()
{
    pthread_mutex_lock(&lock);   //加锁
    if(m_array!=NULL)
        delete [] m_array;

    pthread_mutex_unlock(&lock);  //解锁
    
    pthread_cond_destroy(&cond);
    
    pthread_mutex_destroy(&lock); //摧毁锁
}


//清空队列
template<class T>
void block_queue<T>::clear()
{

    pthread_mutex_lock(&lock);   //加锁
    
    m_size = 0;
    m_front = -1;
    m_back = -1;

    pthread_mutex_unlock(&lock);  //解锁
}

//判断队列是否满了    
template<class T>
bool block_queue<T>::full()
{

    pthread_mutex_lock(&lock);   //加锁

    if(m_size>=m_max_size)
    {
        pthread_mutex_unlock(&lock);
        return true;
    }

    pthread_mutex_unlock(&lock);  //解锁
    return false;
}
//判断队列是否为空
template<class T>
bool block_queue<T>::empty()
{

    pthread_mutex_lock(&lock);   //加锁

    if(m_size==0)
    {
        pthread_mutex_unlock(&lock);
        return true;
    }

    pthread_mutex_unlock(&lock);  //解锁
}
//返回队首元素
template<class T>
bool block_queue<T>::front(T &value)
{

    pthread_mutex_lock(&lock);   //加锁

    if(m_size == 0)
    {
        pthread_mutex_unlock(&lock);
        return false;
    }
    value = m_array(m_front);
    pthread_mutex_unlock(&lock);  //解锁
    return true;
}
//返回队尾元素
template<class T>
bool block_queue<T>::back(T &value)
{

    pthread_mutex_lock(&lock);   //加锁

    if(m_size == 0)
    {
        pthread_mutex_unlock(&lock);
        return false;
    }
    value = m_array(m_back);
    pthread_mutex_unlock(&lock);  //解锁
}
//返回当前队列长度
template<class T>
int block_queue<T>::size()
{
    int tmp = 0;

    pthread_mutex_lock(&lock);   //加锁
    
    tmp= m_size;
    
    pthread_mutex_unlock(&lock);  //解锁

    return tmp;
}
//返回最大尺寸
template<class T>
int block_queue<T>::max_size()
{

    int tmp = 0;

    pthread_mutex_lock(&lock);   //加锁
    
    tmp= m_max_size;
    
    pthread_mutex_unlock(&lock);  //解锁
    
    return tmp;

}
//添加
template<class T>
bool block_queue<T>::push(const T &item)
{
    pthread_mutex_lock(&lock);
    if(m_size >= m_max_size)
    {
        pthread_cond_broadcast(&cond);//队列满，唤醒一个线程
        pthread_mutex_unlock(&lock);
        return false;
    }

    m_back = (m_back + 1);
    m_array[m_back] = item;

    m_size++;

    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&lock);

    return true;
}
//减少
template<class T>
bool block_queue<T>::pop(T &item)
{
    pthread_mutex_lock(&lock);
    while(m_size <= 0)
    {
        if(!pthread_cond_wait(&cond,&lock))
        {
            pthread_mutex_unlock(&lock);
            return false;
        }

        m_front = (m_front + 1);
        item = m_array[m_front];
        m_size--;
        
    }
    pthread_mutex_unlock(&lock);
    return true;
}

#endif

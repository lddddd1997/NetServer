/**
* @file     TimingWheel.cpp
* @brief    时间轮，剔除空闲连接
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <boost/any.hpp>
#include <iostream>
#include <assert.h>
#include "TimingWheel.h"
#include "Logger.h"

ConnectionOnWheel::ConnectionOnWheel(const TcpConnectionWPtr& weak_conn) :
    weak_connection_(weak_conn)
{

}

ConnectionOnWheel::~ConnectionOnWheel() // 时间轮上的Tcp连接的引用计数为0，表示连接空闲到期
{
    TcpConnectionSPtr connection = weak_connection_.lock();
    if(connection != nullptr)
    {
        // LOG_DEBUG << "It is time to shutdown connection" << connection->LocalAddressToString();
        connection->Shutdown(); // 主动关闭连接
    }
}

TimingWheel::TimingWheel(int idle_seconds) :
    connection_buckets_(idle_seconds)
{
    assert(idle_seconds > 0);
    connection_buckets_.resize(idle_seconds); // 设置桶的个数
}

TimingWheel::~TimingWheel()
{

}

void TimingWheel::Start()
{
    std::cout << "Timing wheel start..." << std::endl;
}

void TimingWheel::CommitNewConnection(const TcpConnectionSPtr& sp_tcp_connection) // 提交新连接到时间轮
{
    std::lock_guard<std::mutex> lock(mutex_);
    if(sp_tcp_connection->Connected())
    {
        ConnectionOnWheelSPtr sp_connection_on_wheel(new ConnectionOnWheel(sp_tcp_connection));
        connection_buckets_.back().insert(sp_connection_on_wheel); // 插入到最后一个桶
        ConnectionOnWheelWPtr wp_connection_on_wheel(sp_connection_on_wheel);
        sp_tcp_connection->SetContext(wp_connection_on_wheel);
    }
    else
    {
        std::cout << "Connection is not established" << std::endl;
    }
}

void TimingWheel::Update(const TcpConnectionSPtr& sp_tcp_connection) // 更新连接
{
    ConnectionOnWheelWPtr wp_connection_on_wheel(sp_tcp_connection->Context());
    ConnectionOnWheelSPtr sp_connection_on_wheel(wp_connection_on_wheel.lock());
    if(sp_connection_on_wheel != nullptr)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        connection_buckets_.back().insert(sp_connection_on_wheel);
    }
}

void TimingWheel::TimeLapse()
{
    std::lock_guard<std::mutex> lock(mutex_);
    connection_buckets_.push_back(Bucket());
}

/**
* @file     TimingWheel.h
* @brief    时间轮，剔除空闲连接
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef TIMING_WHEEL_H_
#define TIMING_WHEEL_H_

#include <memory>
#include <boost/circular_buffer.hpp>
#include <unordered_set>
#include "TcpConnection.h"

class ConnectionOnWheel // 时间轮上的Tcp连接
{
public:
    using TcpConnectionWPtr = std::weak_ptr<TcpConnection>;
    using TcpConnectionSPtr = std::shared_ptr<TcpConnection>;

    explicit ConnectionOnWheel(const TcpConnectionWPtr& weak_conn);
    ~ConnectionOnWheel();

private:
    TcpConnectionWPtr weak_connection_;
};

// https://www.cnblogs.com/Solstice/archive/2011/05/04/2036983.html
class TimingWheel // 时间轮
{
public:
    using TcpConnectionSPtr = std::shared_ptr<TcpConnection>;
    using ConnectionOnWheelSPtr = std::shared_ptr<ConnectionOnWheel>;
    using ConnectionOnWheelWPtr = std::weak_ptr<ConnectionOnWheel>;
    using Bucket = std::unordered_set<ConnectionOnWheelSPtr>;
    using ConnectionBuckets = boost::circular_buffer<Bucket>;
    
    TimingWheel() = delete;
    explicit TimingWheel(int idle_seconds = 10);
    ~TimingWheel();

    void Start();
    void CommitNewConnection(const TcpConnectionSPtr& sp_tcp_connection); // 提交新的连接到时间轮的连接桶
    void Update(const TcpConnectionSPtr& sp_tcp_connection); // 连接收到消息后，更新连接到连接桶
    void TimeLapse(); // 定时处理，将轮子上过期的连接桶清除
private:
    ConnectionBuckets connection_buckets_; // 连接桶
};

#endif

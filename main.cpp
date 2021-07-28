#include <ThreadPool.h>
#include <Epoller.h>
#include <Socket.h>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <EventLoopThreadPool.h>
#include <TcpConnection.h>
using namespace std;

using ChannelPtrList = std::vector<Channel*>;

int qq = 0;
void quit(int sig)
{
    qq = 1;
    cout << "-------------------------------------------------------------------" << endl;
}

class EchoConn
{
public:
    EchoConn(int fd) : fd_(fd)
    {

    }
    
    void ReadHandle()
    {
        cout << "cfd = " << fd_ << endl;
        char buf[1024];
        int n = read(fd_, buf, 1024);
        write(STDOUT_FILENO, buf, n);
    }
    void CloseHandle()
    {
        close(fd_);
    }
private:
    int fd_;
};

int main()
{
    signal(SIGINT, quit);
    Socket Socket;
    Socket.SetReuseAddr(true);
    Socket.BindAddress(8888);
    Socket.SetListen();
    
    map<int, Channel*> channel_map;
    ThreadPool Pool(4);
    // EventLoop base_loop;
    EventLoopThreadPool ELPool(nullptr, 4);
    Pool.Start();
    ELPool.Start();
    Channel chan;
    Epoller Epoller;
    chan.SetFd(Socket.Fd());
    chan.SetEvents(EPOLLIN | EPOLLET);
    int save_fd;
    chan.SetReadHandle([&Socket, &Epoller, &channel_map, &save_fd](){
        struct sockaddr_in client_addr;
        int cfd = Socket.Accept(client_addr);
        save_fd = cfd;
        cout << "--------------Client connection " << inet_ntoa(client_addr.sin_addr) << 
        ":" << ntohs(client_addr.sin_port) << " cfd = " << cfd <<  endl;
        channel_map[cfd] = new Channel;
        channel_map[cfd]->SetFd(cfd);
        channel_map[cfd]->SetEvents(EPOLLRDHUP | EPOLLIN | EPOLLET);

        channel_map[cfd]->SetReadHandle([cfd, &channel_map](){
                cout << "cfd = " << cfd << endl;
                char buf[1024];
                int n = read(cfd, buf, 1024);
                if(n == 0)
                    cout << "read close" << endl;
                write(STDOUT_FILENO, buf, n);
            });

        channel_map[cfd]->SetCloseHandle([cfd, &channel_map, &Epoller](){
                Epoller.RemoveChannelFromEpoller(channel_map[cfd]);
                channel_map.erase(cfd);
                // close(cfd);
                cout << "close handle: " << cfd << endl;
                delete channel_map[cfd];
            });
            
        Epoller.CommitChannelToEpoller(channel_map[cfd]);
        });
    
    
    Epoller.CommitChannelToEpoller(&chan);

    // Epoller Epoller;
    // {
    // Channel chan1, chan2;
    // chan1.SetFd(Socket.Fd());
    // chan1.SetEvents(EPOLLIN | EPOLLET);
    // chan1.SetReadHandle([](){cout << "test" << endl;});
    // Epoller.CommitChannelToEpoller(&chan1);
    // }
    
    ChannelPtrList chlist;
    sleep(1);
    signal(SIGPIPE, SIG_IGN);
    while(qq == 0)
    {
        //cout << "start EpollWait" << endl;
        chlist.clear();
        Epoller.EpollWait(1000, chlist);
        //cout << "EpollWait " << chlist.size() << endl;
        if(!chlist.empty())
        {
            for(auto ch : chlist)
            {
                ch->HandleEvents(); // 处理事件
            }
        }
        char buf[1024] = "test\n";
        int n = write(save_fd, buf, 5);
        cout << n << endl;
        perror("write");
        // sleep(1);
    }

    return 0;
}

#include <ThreadPool.h>
#include <Epoller.h>
#include <Socket.h>
#include <iostream>
#include <unistd.h>
#include <signal.h>
using namespace std;

using ChannelList = std::vector<Channel*>;

int qq = 0;
void quit(int sig)
{
    qq = 1;
}

int main()
{
    signal(SIGINT, quit);
    Socket Socket;
    Socket.BindAddress(8888);
    Socket.SetReuseAddr(true);
    Socket.SetListen();
    
    ThreadPool Pool(4);
    Pool.Start();
    Channel chan1, chan2;
    Epoller Epoller;
    chan1.SetFd(Socket.Fd());
    chan1.SetEvents(EPOLLIN | EPOLLET);
    chan1.SetReadHandle([&Socket, &Epoller, &chan2](){
        struct sockaddr_in client_addr;
        int cfd = Socket.Accept(client_addr);
        cout << "--------------Client connection " << inet_ntoa(client_addr.sin_addr) << 
        ":" << ntohs(client_addr.sin_port) << " cfd = " << cfd <<  endl;
        chan2.SetFd(cfd);
        chan2.SetEvents(EPOLLRDHUP | EPOLLIN | EPOLLET);

        chan2.SetReadHandle([cfd](){
                char buf[1024];
                int n = read(cfd, buf, 1024);
                write(STDOUT_FILENO, buf, n);
            });

        chan2.SetCloseHandle([cfd](){
            cout << "close handle" << endl;
            close(cfd);
            // Epoller.remove
            });
            
        Epoller.CommitChannelToEpoller(&chan2);
        });
    
    
    Epoller.CommitChannelToEpoller(&chan1);

    // Epoller Epoller;
    // {
    // Channel chan1, chan2;
    // chan1.SetFd(Socket.Fd());
    // chan1.SetEvents(EPOLLIN | EPOLLET);
    // chan1.SetReadHandle([](){cout << "test" << endl;});
    // Epoller.CommitChannelToEpoller(&chan1);
    // }
    
    ChannelList chlist;
    sleep(1);
    while(qq == 0)
    {
        //cout << "start EpollWait" << endl;
        chlist.clear();
        Epoller.EpollWait(5000, chlist);
        //cout << "EpollWait " << chlist.size() << endl;
        if(!chlist.empty())
        {
            for(auto ch : chlist)
            {
                ch->HandleEvents(); // 处理事件
            }
        }
        // sleep(1);
    }

    return 0;
}

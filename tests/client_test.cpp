#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <memory>
#include <cstring>
#include <signal.h>
using namespace std;

// const char *srv_ip = "127.0.0.1";
// const unsigned short srv_port = 8888;

// unique_ptr<int, void (*)(int *)> recycle_fd(int* fd)
// {
// 	unique_ptr<int, void (*)(int *)> guard(fd,
// 				[](int *p)
// 				{
// 					cout << "close " << *p << endl;
// 					close(*p);
// 				}
// 			);
// 	// return guard; // 编译器优化，不会报错
// 	return move(guard);
// }

int main(int argc, char **argv)
{
	if(argc != 3)
	{
		cout << "Arguments input error" << endl;
		exit(0);
	}
	const char *srv_ip = *(argv + 1);
	const unsigned short srv_port = static_cast<const unsigned short>(stoi(*(argv + 2)));
	if(srv_port > 65535 || srv_port < 1024)
	{
		cout << "Port input error" << endl;
		exit(0);
	}
	
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd == -1)
	{
		perror("Socket:sock_fd");
		exit(0);
	}
	// unique_ptr<int, void (*)(int *)> guard = recycle_fd(&sock_fd);

	struct sockaddr_in srv_addr;
	bzero(&srv_addr, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_addr.s_addr = inet_addr(srv_ip);
	srv_addr.sin_port = htons(srv_port);
	if(connect(sock_fd, reinterpret_cast<const struct sockaddr *>(&srv_addr), sizeof(srv_addr)) == -1)
	{
		perror("Connect");
		exit(0);
	}
	cout << "--------------Connect to server " << srv_ip << ":" << srv_port << " fd = " << sock_fd << endl;

	char buf[1024] = "asdasd";
	write(sock_fd, buf, 10);
	sleep(1);
	/*while(true)
	{
		int n = read(STDIN_FILENO, buf, sizeof(buf));
		n = write(sock_fd, buf, n);

		n = read(sock_fd, buf, sizeof(buf));
		if(n == -1)
		{
			perror("Read");
			exit(0);
		}
		else if(n == 0)
		{
			cout << "Server is closed" << endl;
			exit(0);
		}
		cout << "receive: " << n << endl;
		write(STDOUT_FILENO, buf, n);
	}*/
	// close(sock_fd);
	shutdown(sock_fd, SHUT_RD);
	sleep(5);
	return 0;
}
/**
* @file     HttpServer.cpp
* @brief    http服务器
* @author   lddddd (https://github.com/lddddd1997)
*/
#include "HttpServer.h"
#include "HttpSession.h"
#include "Logger.h"

HttpServer::HttpServer(EventLoop *basic_loop, uint16_t port, int io_thread_num, int worker_thread_num, int idle_seconds) :
    tcp_server_(basic_loop, port, io_thread_num, idle_seconds),
    worker_thread_pool_(worker_thread_num)
{
    tcp_server_.SetMessageCallback(std::bind(&HttpServer::OnMessage, this, std::placeholders::_1, std::placeholders::_2));
    tcp_server_.SetWriteCompleteCallback(std::bind(&HttpServer::OnWriteComplete, this, std::placeholders::_1));
    tcp_server_.SetConnectionCallback(std::bind(&HttpServer::OnNewConnection, this, std::placeholders::_1));
    tcp_server_.SetCloseCallback(std::bind(&HttpServer::OnConnectionClose, this, std::placeholders::_1));
    tcp_server_.SetErrorCallback(std::bind(&HttpServer::OnError, this, std::placeholders::_1));
}

HttpServer::~HttpServer()
{

}

void HttpServer::Start()
{
    LOG_INFO << "Http server start";
    // Logger().EnableAsyncLogging("http", 100 * 1024 * 1024);
    tcp_server_.Start();
    worker_thread_pool_.Start();
}

void HttpServer::OnMessage(const TcpConnectionSPtr& connection, std::string& message)
{
    if(worker_thread_pool_.ThreadsCount() > 0)
    {
        worker_thread_pool_.CommitTaskToPool([this, connection, &message]() // 值捕获，延长连接的生命周期（因为有可能在线程池执行任务时，连接会被关闭）
        {
            OnRequestProcessing(connection, message);
        });
    }
    else
    {
        OnRequestProcessing(connection, message);
    }
}

void HttpServer::OnWriteComplete(const TcpConnectionSPtr& connection)
{

}

void HttpServer::OnNewConnection(const TcpConnectionSPtr& connection)
{

}

void HttpServer::OnConnectionClose(const TcpConnectionSPtr& connection)
{

}

void HttpServer::OnError(const TcpConnectionSPtr& connection)
{

}

void HttpServer::OnRequestProcessing(const TcpConnectionSPtr& connection, std::string& message)
{
    std::string msg;
    msg.swap(message);
    // LOG_DEBUG << msg;
    HttpContext http_context;
    if(!http_context.ParseRequest(msg, Timestamp::Now())) // 解析消息
    {
        LOG_DEBUG << "Parse error";
        connection->Send("HTTP/1.1 400 Bad Request\r\n\r\n"); // 处理完成，投递到连接所处的io线程
        connection->Shutdown();
        return ;
    }
    if(http_context.GotAll())
    {
        // std::cout << "----------------------------------------------" << std::endl;
        // std::cout << "method: " << http_context.Request().Method() << '|' << std::endl;
        // std::cout << "path: " << http_context.Request().Path() << '|' << std::endl;
        // std::cout << "version: " << http_context.Request().Version() << '|' << std::endl;
        // std::cout << "query: " << http_context.Request().Query() << '|' << std::endl;
        // std::cout << "headers: " << http_context.Request().Headers().size() << std::endl;
        // for(auto& it : http_context.Request().Headers())
        // {
        //     std::cout << it.first << ": " << it.second << '|' << std::endl;
        // }
        // std::cout << "----------------------------------------------" << std::endl;
        // LOG_INFO << "[ " << http_context.Request().Path() << " ]" << " request from " << connection->LocalAddressToString();
        bool close = (http_context.Request().Header("Connection") == "close") || 
                        (http_context.Request().Version() == HttpRequest::HTTP_10 &&
                        http_context.Request().Header("Connection") != "Keep-Alive");
        HttpResponse http_response(close);
        if(http_context.Request().Path() == "/")
        {
            http_response.SetStatusCode(HttpResponse::OK_200);
            http_response.SetStatusMessage("OK");
            http_response.SetContentType("text/html");
            http_response.AddHeader("Server", "lddddd");
            http_response.SetBody("<html><head><title>This is title</title></head>"
                                    "<body><h1>Hello</h1>Now is " +
                                    Timestamp::Now().ToFormattedString() +
                                    "</body></html>");
        }
        else if(http_context.Request().Path() == "/favicon.ico")
        {
            http_response.SetStatusCode(HttpResponse::OK_200);
            http_response.SetStatusMessage("OK");
            http_response.SetContentType("image/png");
            http_response.AddHeader("Server", "lddddd");
            // http_response.SetBody(std::string(favicon, sizeof(favicon)));
        }
        else if(http_context.Request().Path() == "/hello")
        {
            http_response.SetStatusCode(HttpResponse::OK_200);
            http_response.SetStatusMessage("OK");
            http_response.SetContentType("text/plain");
            http_response.AddHeader("Server", "lddddd");
            http_response.SetBody("The current UTC time of server is " + Timestamp::Now().ToFormattedString());
        }
        else
        {
            http_response.SetStatusCode(HttpResponse::NOT_FOUND_404);
            http_response.SetStatusMessage("Not Found");
            http_response.SetCloseConnection(true);
        }
        std::string output;
        http_response.AppendToSendBuffer(output);
        // LOG_DEBUG << output;
        connection->Send(output); // 处理完成，投递到连接所处的io线程
        if(http_response.CloseConnection())
        {
            connection->Shutdown();
        }
    }
}

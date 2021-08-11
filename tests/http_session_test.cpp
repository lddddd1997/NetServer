#include "../NetServer/HttpSession.h"
#include <iostream>
using namespace std;

string request_line("GET / HTTP/1.1\r\n");
string request_headers1("Host: 192.168.199.1:8888\r\n");
string request_headers2("Connection: keep-alive\r\n");

string request_headers3("Upgrade-Insecure-Requests: 1\r\n");
string request_headers4("Accept-Encoding: gzip, deflate\r\n");
string request_headers5("Accept-Language: zh-CN,zh;q=0.9\r\n");
string request_headers6("User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/92.0.4515.131 Safari/537.36\r\n");
string request_body("\r\nbody test");

string request = request_line + 
                    request_headers1 + 
                    request_headers2 + 
                    request_headers3 + 
                    request_headers4 +
                    request_headers5 + 
                    request_headers6 + request_body;

int main()
{
    HttpContext http_context;
    cout << "parse result: " << http_context.ParseRequest(request, Timestamp::Now()) << endl;
    cout << "Request string left: " << request.size() << endl;
    cout << "method: " << http_context.Request().Method() << '|' << endl;
    cout << "path: " << http_context.Request().Path() << '|' << endl;
    cout << "version: " << http_context.Request().Version() << '|' << endl;
    cout << "query: " << http_context.Request().Query() << '|' << endl;
    cout << "headers: " << http_context.Request().Headers().size() << endl;
    for(auto& it : http_context.Request().Headers())
    {
        cout << it.first << ": " << it.second << '|' << endl;
    }
    bool close = (http_context.Request().Header("Connection") == "close") || 
                (http_context.Request().Version() == HttpRequest::HTTP_10 && (http_context.Request().Header("Connection") != "Keep-Alive"));
    cout << endl << endl;
    HttpResponse http_response(close);
    if(http_context.Request().Path() == "/")
    {
        http_response.SetStatusCode(HttpResponse::OK_200);
        http_response.SetStatusMessage("OK");
        http_response.SetContentType("text/html");
        http_response.AddHeader("Header", "Test");
        http_response.SetBody("Server UTC time is" + Timestamp::Now().ToFormattedString());
    }
    string output;
    http_response.AppendToSendBuffer(output);
    cout << output << endl;
    cout << "Finished" << endl;
    return 0;
}

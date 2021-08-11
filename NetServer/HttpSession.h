/**
* @file     HttpSession.h
* @brief    http会话
* @author   lddddd (https://github.com/lddddd1997)
*/
#ifndef HTTP_SESSION_H_
#define HTTP_SESSION_H_

#include <string>
#include <map>
#include "Timestamp.h"

class HttpRequest
{
public:
    HttpRequest();
    ~HttpRequest();

    enum MethodEnum
    {
        INVALID,
        GET, // 请求指定的页面信息，并返回实体主体
        POST, // 类似于GET请求，只不过返回的响应中没有具体的内容，用于获取报头
        HEAD, // 向指定资源提交数据进行处理（例如提交表单或上传文件）。数据被包含在请求体中。POST请求可能会导致新的资源的建立或已有资源的修改
        PUT, // 向指定资源位置上传其最新内容
        DELETE, // 请求服务器删除Request-URL所标识的资源
        CONNECT, // HTTP/1.1协议中预留给能够将连接改为管道方式的代理服务器
        OPTIONS, // 返回服务器针对特定资源所支持的HTTP请求方法，也可以利用向web服务器发送‘*’的请求来测试服务器的功能性
        TRACE // 回显服务器收到的请求，主要用于测试或诊断
    };
    enum VersionEnum
    {
        UNKNOWN, HTTP_10, HTTP_11
    };
    void SetVersion(VersionEnum v)
    {
        version_ = v;
    }
    VersionEnum Version() const
    {
        return version_;
    }
    bool SetMethod(std::string::const_iterator start, std::string::const_iterator end);
    MethodEnum Method()
    {
        return method_;
    }
    void SetPath(std::string::const_iterator start, std::string::const_iterator end)
    {
        path_.assign(start, end);
    }
    const std::string& Path() const
    {
        return path_;
    }
    void SetQuery(std::string::const_iterator start, std::string::const_iterator end)
    {
        query_.assign(start, end);
    }
    const std::string& Query() const
    {
        return query_;
    }
    void SetReceiveTime(Timestamp t)
    {
        receive_time_ = t;
    }
    Timestamp ReceiveTime() const
    {
        return receive_time_;
    }
    void AddHeader(std::string::const_iterator start, std::string::const_iterator colon, std::string::const_iterator end)
    {
        std::string field(start, colon);
        ++colon;
        while (colon < end && isspace(*colon))
        {
            ++colon;
        }
        std::string value(colon, end);
        while (!value.empty() && isspace(value[value.size()-1]))
        {
            value.resize(value.size()-1);
        }
        headers_[field] = value;
    }
    std::string Header(const std::string& field) const
    {
        std::string result;
        std::map<std::string, std::string>::const_iterator it = headers_.find(field);
        if (it != headers_.end())
        {
            result = it->second;
        }
        return result;
    }
    const std::map<std::string, std::string>& Headers() const
    {
        return headers_;
    }
    void Swap(HttpRequest& that)
    {
        std::swap(method_, that.method_);
        std::swap(version_, that.version_);
        path_.swap(that.path_);
        query_.swap(that.query_);
        receive_time_.Swap(that.receive_time_);
        headers_.swap(that.headers_);
    }

private:
    MethodEnum method_;
    VersionEnum version_;
    std::string path_;
    std::string query_;
    Timestamp receive_time_;
    std::map<std::string, std::string> headers_;
};

class HttpResponse
{
public:
    explicit HttpResponse(bool close);
    ~HttpResponse();

    // 1** 信息，服务器收到请求，需要请求者继续执行操作
    // 2** 成功，操作成功接受并处理
    // 3** 重定向，需要进一步的操作以完成请求
    // 4** 客户端错误，请求包含语法错误或无法完成请求
    // 5** 服务器错误，服务器在处理请求的过程中发生了错误
    enum HttpStatusCodeEnum
    {
        UNKNOWN,
        OK_200 = 200,
        MOVED_PERMANENTLY_301 = 301,
        BAD_REQUEST_400 = 400,
        NOT_FOUND_404 = 404,
    };

    void SetStatusCode(HttpStatusCodeEnum code)
    {
        status_code_ = code;
    }
    void SetStatusMessage(const std::string& message)
    {
        status_message_ = message;
    }
    void SetCloseConnection(bool on)
    {
        close_connection_ = on;
    }

    bool CloseConnection() const
    {
        return close_connection_;
    }

    void SetContentType(const std::string& content_type)
    {
        AddHeader("Content-Type", content_type);
    }
    void AddHeader(const std::string& key, const std::string& value)
    {
        headers_[key] = value;
    }
    void SetBody(const std::string& body)
    {
        body_ = body;
    }
    void AppendToSendBuffer(std::string& output) const;

private:
    bool close_connection_;
    HttpStatusCodeEnum status_code_;
    std::string status_message_;
    std::map<std::string, std::string> headers_;
    std::string body_;
};

class HttpContext
{
public:
    HttpContext();
    ~HttpContext();

    enum HttpRequestParseStateEnum
    {
        REQUEST_LINE, // 请求行
        REQUEST_HEADERS, // 请求头
        REQUEST_BODY, // 请求体
        GOT_ALL
    };

    bool ParseRequest(std::string& message, Timestamp reveive_time);
    bool GotAll() const
    {
        return state_ == GOT_ALL;
    }
    void Reset()
    {
        state_ = REQUEST_LINE;
        HttpRequest().Swap(request_);
    }
    const HttpRequest& Requset() const
    {
        return request_;
    }
    HttpRequest& Request()
    {
        return request_;
    }

private:
    HttpRequestParseStateEnum state_;
    HttpRequest request_;

    bool ProcessRequestLine(const std::string& request_line);
};

#endif

/**
* @file     HttpSession.cpp
* @brief    http会话
* @author   lddddd (https://github.com/lddddd1997)
*/
#include <algorithm>
#include "HttpSession.h"

HttpRequest::HttpRequest() :
    method_(INVALID),
    version_(UNKNOWN)
{

}

HttpRequest::~HttpRequest()
{

}

bool HttpRequest::SetMethod(std::string::const_iterator start, std::string::const_iterator end)
{
    std::string m(start, end);
    if(m == "GET")
    {
        method_ = GET;
    }
    else if(m == "POST")
    {
        method_ = POST;
    }
    else if(m == "HEAD")
    {
        method_ = HEAD;
    }
    else if(m == "PUT")
    {
        method_ = PUT;
    }
    else if(m == "DELETE")
    {
        method_ = DELETE;
    }
    else if(m == "CONNECT")
    {
        method_ = CONNECT;
    }
    else if(m == "OPTIONS")
    {
        method_ = OPTIONS;
    }
    else if(m == "TRACE")
    {
        method_ = TRACE;
    }
    else
    {
        method_ = INVALID;
    }
    return method_ != INVALID;
}

HttpResponse::HttpResponse(bool close) :
    close_connection_(close),
    status_code_(UNKNOWN)
{

}

HttpResponse::~HttpResponse()
{

}

void HttpResponse::AppendToSendBuffer(std::string& output) const
{
    char buf[32];
    snprintf(buf, sizeof(buf), "HTTP/1.1 %d ", status_code_);
    output.append(buf);
    output.append(status_message_);
    output.append("\r\n");
    if(close_connection_)
    {
        output.append("Connection: close\r\n");
    }
    else
    {
        snprintf(buf, sizeof(buf), "Content-Length: %zd\r\n", body_.size());
        output.append(buf);
        output.append("Connection: Keep-Alive\r\n");
    }
    for (const auto& header : headers_)
    {
        output.append(header.first);
        output.append(": ");
        output.append(header.second);
        output.append("\r\n");
    }
    output.append("\r\n");
    output.append(body_);
}

HttpContext::HttpContext() :
    state_(REQUEST_LINE)
{

}

HttpContext::~HttpContext()
{

}

bool HttpContext::ParseRequest(std::string& message, Timestamp reveive_time)
{
    bool ok = false;
    bool has_more = true;
    std::string crlf("\r\n");
    while(has_more)
    {
        if(state_ == REQUEST_LINE)
        {
            std::string::size_type find_crlf = message.find(crlf, 0);
            if(find_crlf != std::string::npos)
            {
                std::string request_line(message.begin(), message.begin() + find_crlf);
                ok = ProcessRequestLine(request_line);
                if(ok)
                {
                    request_.SetReceiveTime(reveive_time);
                    std::string remain_message(message.begin() + find_crlf + 2, message.end());
                    message = std::move(remain_message);
                    state_ = REQUEST_HEADERS;
                }
                else
                {
                    has_more = false;
                }
            }
            else
            {
                has_more = false;
            }
        }
        else if(state_ == REQUEST_HEADERS)
        {
            std::string::size_type find_crlf = message.find(crlf, 0);
            if(find_crlf != std::string::npos)
            {
                std::string::const_iterator find_colon = std::find(message.begin(), message.begin() + find_crlf, ':');
                if(find_colon != message.begin() + find_crlf)
                {
                    request_.AddHeader(message.begin(), find_colon, message.begin() + find_crlf);
                }
                else
                {
                    state_ = GOT_ALL;
                    has_more = false;
                }
                std::string remain_message(message.begin() + find_crlf + 2, message.end());
                message = std::move(remain_message);
            }
            else
            {
                has_more = false;
            }
        }
        else if(state_ == REQUEST_BODY)
        {
            has_more = false;
        }
        else
        {
            has_more = false;
        }
    }
    return ok;
}

bool HttpContext::ProcessRequestLine(const std::string& request_line)
{
    bool succeed = false;
    std::string::const_iterator find_space = std::find(request_line.begin(), request_line.end(), ' ');
    if(find_space != request_line.end() && request_.SetMethod(request_line.begin(), find_space))
    {
        std::string::const_iterator start = find_space + 1;
        find_space = std::find(start, request_line.end(), ' ');
        if(find_space != request_line.end())
        {
            std::string::const_iterator find_question = std::find(start, find_space, '?');
            if(find_question != find_space)
            {
                request_.SetPath(start, find_question);
                request_.SetQuery(find_question, find_space);
            }
            else
            {
                request_.SetPath(start, find_space);
            }
            start = find_space + 1;
            if(request_line.end() - start == 8 && std::string(start, request_line.end() - 1) == "HTTP/1.")
            {
                succeed = true;
                if(*(request_line.end() - 1) == '1')
                {
                    request_.SetVersion(HttpRequest::HTTP_11);
                }
                else if(*(request_line.end() - 1) == '0')
                {
                    request_.SetVersion(HttpRequest::HTTP_10);
                }
                else
                {
                    succeed = false;
                }
            }
        }
    }
    return succeed;
}

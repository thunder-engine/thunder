#ifndef WEBREQUEST_H
#define WEBREQUEST_H

#include <url.h>

#include <network.h>

class TcpSocket;

class NETWORK_EXPORT WebRequest {
    A_GENERIC(WebRequest)

    A_METHODS(
        A_METHOD(int, WebRequest::errorCode),
        A_METHOD(bool, WebRequest::isDone),
        A_METHOD(int, WebRequest::downloadedBytes),
        A_METHOD(float, WebRequest::downloadProgress),
        A_METHOD(std::string, WebRequest::text),
        A_METHOD(void, WebRequest::setHeader),
        A_METHOD(void, WebRequest::send),
        A_STATIC(WebRequest *, WebRequest::get)
    )

public:
    typedef std::list<std::pair<std::string, std::string>> HeaderPairs;

public:
    WebRequest();

    ~WebRequest();

    bool operator== (const WebRequest &right) const;

    int errorCode() const;

    bool isDone();

    int downloadedBytes() const;

    float downloadProgress();

    std::string text() const;
    const uint8_t *data() const;

    void setHeader(const std::string &key, const std::string &value);

    void send();

    static WebRequest *get(const std::string &url);

protected:
    void readAnswer();

private:
    std::map<std::string, std::string> m_fields;

    std::list<std::pair<std::string, std::string>> m_header;

    TcpSocket *m_socket;

    ByteArray m_content;

    ByteArray m_sub(1024);

    std::string m_url;

    std::string m_operation;

    size_t m_downloadTotal;

    size_t m_downloadCurrent;

    int m_code;

    int m_state;

};

#endif // WEBREQUEST_H

#include "webrequest.h"

#include "tcpsocket.h"

enum State {
    Initial = 0,
    Connecting,
    SendingHeader,
    ReadingCode,
    ReadingHeader,
    ReadingContent,
    Done,
    Error
};

/*!
    \class WebRequest
    \brief The WebRequest class provides a mechanism to send HTTP requests.
    \inmodule Network

    The WebRequest class provides a mechanism to send HTTP requests (e.g., GET requests) and handle the server's response.
    It manages the connection, sends headers, and processes the response, including handling HTTP status codes, headers, and content.
    This class supports both regular HTTP and HTTPS protocols.
*/

WebRequest::WebRequest() :
        m_socket(nullptr),
        m_downloadTotal(0),
        m_downloadCurrent(0),
        m_code(0),
        m_state(State::Initial) {

    m_sub.resize(1024);

}

WebRequest::~WebRequest() {
    delete m_socket;
}
/*!
    Compares WebRequest with \a right object for equality based on their headers.
    Returns true if requests are equal; otherwise returns false.
*/
bool WebRequest::operator== (const WebRequest &right) const {
    return m_fields == right.m_fields;
}
/*!
    Returns the HTTP status code from the server response.
*/
int WebRequest::errorCode() const {
    return m_code;
}
/*!
     Checks if the HTTP request has been completed (i.e., response fully received or some error happened).
*/
bool WebRequest::isDone() {
    if(m_state >= State::Done) {
        return true;
    } else {
        readAnswer();
    }
    return false;
}
/*!
    Returns the content of the response as a std::string
*/
std::string WebRequest::text() const {
    std::string result;
    std::copy(m_content.begin(), m_content.end(), std::back_inserter(result));
    return result;
}
/*!
     Returns the raw response data as a pointer to a uint8_t array.
*/
const uint8_t *WebRequest::data() const {
    return m_content.data();
}
/*!
    Returns the number of bytes downloaded so far in the response.
*/
int WebRequest::downloadedBytes() const {
    return m_downloadCurrent;
}
/*!
    Returns the progress of the download as a percentage of the total content size as a float between 0.0f and 1.0f.
*/
float WebRequest::downloadProgress() {
    if(m_downloadTotal != 0) {
        return static_cast<float>(m_downloadCurrent) / static_cast<float>(m_downloadTotal);
    }
    return 0.0f;
}
/*!
    Sends the HTTP request by constructing the appropriate headers, establishing a socket connection, and writing the request to the server.
*/
void WebRequest::send() {
    Url url(m_url);
    std::string host(url.host());
    std::string scheme(url.scheme());
    std::string path(url.path());
    std::string headerData(m_operation + " " + (path.empty() ? "/" : path) + " HTTP/1.1\r\n");

    headerData += "Host: " + host + "\r\n";
    headerData += "Connection: keep-alive\r\n";

    for(auto &it : m_header) {
        headerData += it.first + ": " + it.second + "\r\n";
    }

    headerData += "\r\n";

    bool useSSL = false;
    int port = 80;
    if(scheme == "https") {
        port = 443;
        useSSL = true;
    }

    if(m_socket) {
        delete m_socket;
    }
    m_socket = new TcpSocket(useSSL);

    m_state = State::Connecting;
    if(m_socket->connect(NetworkAddress(host, port))) {
        // Prepare HTTP request
        m_state = State::SendingHeader;
        ByteArray data;
        std::copy(headerData.begin(), headerData.end(), std::back_inserter(data));
        if(m_socket->write(data) == headerData.size()) {
            m_content.clear();

            m_state = State::ReadingCode;
            readAnswer();
        }
    } else {
        m_state = State::Error;
    }
}
/*!
    Creates a WebRequest object configured for a GET request with the specified \a url.
*/
WebRequest *WebRequest::get(const std::string &url) {
    WebRequest *result = new WebRequest();
    result->m_url = url;
    result->m_operation = "GET";

    return result;
}
/*!
    Adds a custom header to the request.
    Each field must contain \a key as a header field name and \a value.
*/
void WebRequest::setHeader(const std::string &key, const std::string &value) {
    m_header.push_back(std::make_pair(key, value));
}
/*!
    Reads the server's response, handling the status code, headers, and content.
    This function processes the response in chunks and updates the internal state accordingly.
*/
void WebRequest::readAnswer() {
    if(m_socket->isDataAvailable()) {
        uint64_t size = m_socket->read(m_sub);
        if(size > 0) {
            for(uint64_t i = 0; i < size; i++) {
                switch(m_state) {
                case State::ReadingCode: {
                    if(m_sub[i] == ' ') {
                        i++;
                        std::string code;
                        while(i < size) {
                            if(m_sub[i] == ' ') {
                                break;
                            }

                            code.push_back(m_sub[i]);
                            i++;
                        }
                        while(m_sub[i] != '\n') {
                            i++;
                        }
                        char *end;
                        m_code = strtol(code.c_str(), &end, 10);
                        m_state = State::ReadingHeader;
                    }
                } break;
                case State::ReadingHeader: {
                    std::string key;
                    std::string value;
                    bool readKey = true;
                    while(i < size) {
                        if(m_sub[i] != '\r') {
                            if(readKey) {
                                if(m_sub[i] != ':') {
                                    key.push_back(m_sub[i]);
                                } else {
                                    i++;
                                    readKey = false;
                                }
                            } else {
                                value.push_back(m_sub[i]);
                            }
                        } else {
                            i++;
                            break;
                        }
                        i++;
                    }

                    if(!key.empty() && !value.empty()) {
                        m_fields[key] = value;
                    } else {
                        auto it = m_fields.find("Content-Length");
                        if(it != m_fields.end()) {
                            char *end;
                            m_downloadTotal = strtol(it->second.c_str(), &end, 10);
                            m_content.resize(m_downloadTotal);
                        }
                        m_state = State::ReadingContent;
                    }
                } break;
                case State::ReadingContent: {
                    size_t count = size - i;
                    memcpy(&m_content[m_downloadCurrent], &m_sub[i], count);
                    m_downloadCurrent += count;
                    if(m_content.size() == m_downloadCurrent) {
                        m_state = State::Done;
                    }
                    return;
                }
                default: break;
                }
            }
        } else {
            m_state = State::Done;
        }
    }
}

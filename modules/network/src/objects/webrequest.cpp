#include "webrequest.h"

#include "tcpsocket.h"

enum State {
    Initial = 0,
    Connecting,
    SendingHeader,
    ReadingCode,
    ReadingHeader,
    ReadingContent,
    Done
};

WebRequest::WebRequest() :
        m_socket(nullptr),
        m_downloadTotal(0),
        m_downloadCurrent(0),
        m_code(0),
        m_state(State::Initial) {

}

WebRequest::~WebRequest() {
    delete m_socket;
}

bool WebRequest::operator== (const WebRequest &right) const {
    return m_fields == right.m_fields;
}

int WebRequest::errorCode() const {
    return m_code;
}

bool WebRequest::isDone() {
    if(m_state == State::Done) {
        return true;
    } else {
        readAnswer();
    }
    return false;
}

std::string WebRequest::text() const {
    std::string result;
    std::copy(m_content.begin(), m_content.end(), std::back_inserter(result));
    return result;
}

const uint8_t *WebRequest::data() const {
    return m_content.data();
}

int WebRequest::downloadedBytes() const {
    return m_downloadCurrent;
}

float WebRequest::downloadProgress() {
    if(m_downloadTotal != 0) {
        return static_cast<float>(m_downloadCurrent) / static_cast<float>(m_downloadTotal);
    }
    return 0;
}

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
    }
}

WebRequest *WebRequest::get(const std::string &url) {
    WebRequest *result = new WebRequest();
    result->m_url = url;
    result->m_operation = "GET";

    return result;
}

void WebRequest::setHeader(const std::string &key, const std::string &value) {
    m_header.push_back(std::make_pair(key, value));
}

void WebRequest::readAnswer() {
    ByteArray sub(1024);

    while(m_socket->isDataAvailable()) {
        uint64_t size = m_socket->read(sub);
        if(size > 0) {
            for(uint64_t i = 0; i < size; i++) {
                switch(m_state) {
                case State::ReadingCode: {
                    if(sub[i] == ' ') {
                        i++;
                        std::string code;
                        while(i < size) {
                            if(sub[i] == ' ') {
                                break;
                            }

                            code.push_back(sub[i]);
                            i++;
                        }
                        while(sub[i] != '\n') {
                            i++;
                        }
                        char *end;
                        m_code = strtol(code.c_str(), &end, 10);
                        m_state = State::ReadingHeader;
                    }
                } break;
                case State::ReadingHeader: {
                    static int index = 0;

                    std::string key;
                    std::string value;
                    bool readKey = true;
                    while(i < size) {
                        if(sub[i] != '\r') {
                            if(readKey) {
                                if(sub[i] != ':') {
                                    key.push_back(sub[i]);
                                } else {
                                    i++;
                                    readKey = false;
                                }
                            } else {
                                value.push_back(sub[i]);
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

                    index++;
                } break;
                case State::ReadingContent: {
                    size_t count = size - i;
                    memcpy(&m_content[m_downloadCurrent], &sub[i], count);
                    m_downloadCurrent += count;
                    if(m_content.size() == m_downloadCurrent) {
                        m_state = State::Done;
                    }

                    return;
                } break;
                default: break;
                }
            }
        }
    }
}

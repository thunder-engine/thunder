#ifndef URI_H
#define URI_H

#include <string>
#include <regex>

#include <global.h>

using namespace std;

class NEXT_LIBRARY_EXPORT Uri {
public:
    Uri(const string &uri);

    string scheme() const;
    string host() const;
    string path() const;
    string query() const;
    string fragment() const;
    string dir() const;
    string name() const;
    string baseName() const;
    string suffix() const;

private:
    string m_uri;

    smatch m_result;

};

#endif // URI_H

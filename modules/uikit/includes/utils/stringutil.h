#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <string>
#include <vector>

// string switch
constexpr unsigned long long strhash(const char* str, int h = 0)
{
	return !str[h] ? 5381 : (strhash(str, h + 1) * 33) ^ str[h];
}

class StringUtil {
public:
	StringUtil();
	virtual ~StringUtil();

	static const char* ws;

	static std::string longlong2str(long long in);
	static std::string int2str(int in);
	static int str2int(const std::string &in);
	static bool startWith(std::string &str,std::string &strWith);
	static bool endWith(std::string &str,std::string &strWith);
	static std::string tolower(const std::string &str);
	static std::string toupper(const std::string &str);
	static bool contains(const std::string str1,const std::string str2);
	static std::string tostring(long long in);
	static void replace(std::string &srcStr, const std::string &findStr, const std::string &replaceStr );
	static std::string& rtrim(std::string& s, const char* t = ws);
	static std::string& ltrim(std::string& s, const char* t = ws);
	static std::string& trim(std::string& s, const char* t = ws);
    static std::vector<std::string> split(const std::string& , char c);
	static std::vector<std::string> splitButSkipBrackets(const std::string& s, char separator);
	static std::string join(std::vector<std::string>& v, char separator);
    static std::string deletechar(const std::string&, char);
};

#endif /* STRINGUTIL_H */

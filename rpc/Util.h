
#include <string>
#include <list>

namespace rpc {
/*
enum JSON_RPC_ERROR {
	SUCCESS = 0,
	INVALID_REQUEST = -32600,
	UNKNOW_METHOD = -32601,
	PARAMS_ERROR = -32602,
	NETWORK_ERROR = -32603,
	INTERNAL_ERROR = -500,
};
*/
/*safe pointer free*/
#define event_safe_free(_var, _freefn) do { \
        _freefn((_var));                    \
        (_var) = NULL;                      \
} while (0)

/*split host:port*/
void split(const std::string& s, std::list<std::string>& v, const std::string& c); /*
void split(const std::string& s, std::list<std::string>& v, const std::string& c) {
	std::string::size_type pos1 = 0;
	std::string::size_type pos2 = s.find(c);
	
	while (std::string::npos != pos2) {
		v.push_back(s.substr(pos1, pos2 - pos1));
		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}

	if (pos1 != s.length()) {
		v.push_back(s.substr(pos1));
	}
}
*/
class noncopyable {
protected:
	noncopyable() = default;
	~noncopyable() = default;

private:
	noncopyable(const noncopyable&) = delete;
	const noncopyable& operator = (const noncopyable&);
};/*end noncopyable*/

}

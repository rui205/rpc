#include "Util.h"

namespace rpc {

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

}/*end namespace rpc*/

#include <functional>

namespace rpc {

class ThreadPoolInterface {
public:
    virtual ~ThreadPoolInterface() {}
    virtual void add(const std::function<void(void*)>& callback) = 0;
};

}/*end namespace rpc*/
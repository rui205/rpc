#ifndef __DATABASE_ACCESS_OBJECT_H__
#define __DATABASE_ACCESS_OBJECT_H__

#include <map>
#include <vector>
#include <unordered_map>
#include <mysql/mysql.h>

//#include "Util.h"
#include "hiredis/hiredis.h"
#include "libevent/event2/util.h"
#include "libevent/event2/event.h"
#include "hiredis/adapters/libevent.h"


namespace rpc {

/*redis sync interface*/
redisContext* redisConnect(const char* host, const int port);
int redisClose(redisContext* context);
int redisExpire(redisContext* context, const char* key, const unsigned int sec);
int redisMulti(redisContext* context);
int redisExec(redisContext* context);
int redisSet(redisContext* context, const char* key, const char* value);
int redisGet(redisContext* context, const char* key, char* value);
int redisDel(redisContext* context, const char* key);
int redisHSet(redisContext* context, const char* key, const char* field, const char* value);
int redisHGet(redisContext* context, const char* key, const char* field, char* value);
int redisHDel(redisContext* context, const char* key, const char* field);
std::vector<std::string> redisHGetall(redisContext* context, const char* key);
std::unordered_map<std::string, std::string> redisHGetall2(redisContext* context, const char* key);
int redisLPush(redisContext* context, const char* key, const char* value);
int redisLPop(redisContext* context, const char* key, char* value);
int redisRPush(redisContext* context, const char* key, const char* value);
int redisRPop(redisContext* context, const char* key, char* value);
int redisLRem(redisContext* context, const char* key, int top, const char* value);
std::vector<std::string> redisLRange(redisContext* context, const char* key, int beg, int end);

/*libevent redis async interface*/
class Redis {
public:
    Redis(struct redisAsyncContext* ctx, struct event_base* base);
    ~Redis();
    static void updateTimer(struct event* timer, int sec);
    static void connectCallback(const struct redisAsyncContext* ctx, int status);
    static void disConnectCallback(const struct redisAsyncContext* ctx, int status);
    static void reConnectCallback(int fd, short events, void* arg);
    static void checkHealthCallback(int fd, short events, void* arg);
    static void commandCallback(struct redisAsyncContext* ctx, void* reply, void* arg);
    static void doCommandCallback(struct redisAsyncContext* ctx, void* reply, void* arg);

public:
    int status_;
    redisAsyncContext* context_;
    struct event check_;
    struct event_base* base_;
};
   
typedef std::map<std::string, std::string> string_array_t;

class MySQLStatement;

class MySQL {
public:
    MySQL(struct event_base* base,
        const char* host, const char* user, 
        const char* passwd, const char* database,
        int port);

    ~MySQL();

    MySQLStatement* prepare(const char* sql);
    int query(std::string stmt);
    char** fetch();
    void resultFree();

private:
    int MySQLConnect();
//    static void mysqlcb(int fd, short events, void* arg); 
//    static void timeoutcb(int fd, short events, void* arg);

public:
    MYSQL* mysql_;
    MYSQL_RES* res;
    std::string errorMessage_;
//    struct event_base* base_;                           
//    struct event* eventTimeout_;
//    struct event* eventMysql_;
//    int events_;
    const char* host_;
    const char* user_;
    const char* passwd_;
    const char* database_;
    int port_;
};

class MySQLStatement {
public:
    MySQLStatement(MYSQL* mysql, const char* sql);
    ~MySQLStatement();

public:
    int bind(int pos, void* value, int length, enum_field_types type);
    int params(int pos, void* value, enum_field_types type, my_bool flag = 1);
    int execute();
    int fetch();
    int allFetch();
    int columnFetch();
    int rows();
    int cols();

public:
    MYSQL* mysql_;
    MYSQL_STMT* stmt_;
    MYSQL_BIND* args_;
    std::vector<MYSQL_BIND> values_;
    std::string errorMessage_;
    bool error_;
};

}/*end namespace rpc*/

#endif

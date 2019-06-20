#include "Dao.h"

#include "glog/logging.h"

namespace rpc {

/*test*/
//std::string redisAddr = "127.0.0.1";
//unsigned int redisPort = 6379;

const char* CHECK_HEALTH_CMD = "SET NAME WANGPENG";

redisContext* redisConnect(const char* host, const int port) {
	if (host == NULL || port <= 0) {
		return NULL;
	}

	redisContext* c = redisConnect(host, port)
	if (c == NULL || e->err) {
		LOG(ERROR) << "redisConnect met error: " << c->errstr;
		return NULL;
	}

	return c;
}

int redisClose(redisContext* context) {
	if (context != NULL) {
		redisFree(context);
		context = NULL;
		return 0;
	}

	return -1;
}

int redisExpire(redisContext* context, const char* key, const unsigned int sec) {
	int rc = -1;

	if (context == NULL || key == NULL) {
		return rc;
	}

	redisReply* reply = (redisReply*)(redisCommand(context, "EXPIRE %s %u", key, sec));
	if (reply != NULL &&
		reply->type == REDIS_REPLY_INTEGER &&
		reply->integer == 1) {
		rc = 0;
	}

	if (reply != NULL) {
		freeReplyObject(reply);
		reply = NULL;
	}

	return rc;
}

int redisMulti(redisContext* context) {
	int rc = -1;

	if (context == NULL) {
		return rc;
	}

	redisReply* reply = (redisReply*)(redisCommand(context, "MULTI"));
	if (reply != NULL &&
		reply->str != NULL &&
		reply->type == REDIS_REPLY_STATUS &&
		!strcmp(reply->str, "OK")) {
		rc = 0;
	}

	if (reply != NULL) {
		freeReplyObject(reply);
		reply = NULL;
	}

	return rc;
}

int redisExec(redisContext* context) {
	int rc = -1;
	
	if (context == NULL) {
		return rc;
	}

	redisReply* reply = (redisReply*)(redisCommand(context, "EXEC"));
	if (reply != NULL &&
		reply->elements != 0 &&
		reply->type == REDIS_REPLY_ARRAY) {
		rc = 0;
	}

	if (reply != NULL) {
		freeReplyObject(reply);
		reply = NULL;
	}

	return rc;
}

int redisSet(redisContext* context, const char* key, const char* value) {
	int rc = -1;

	if (context == NULL || key == NULL || value == NULL) {
		return rc;
	}

	redisReply* reply = (redisReply*)(redisCommand(context, "SET %s %s", key, value));
	if (reply != NULL &&
		reply->str != NULL &&
		reply->type == REDIS_REPLY_STATUS &&
		!strcmp(reply->str, "OK")) {
		rc = 0;
	}

	if (reply != NULL) {
		freeReplyObject(reply);
		reply = NULL;
	}

	return rc;
}

int redisGet(redisContext* context, const char* key, char* value) {
	int rc = -1;

	if (context == NULL || key == NULL || value == NULL) {
		return NULL;
	}

    redisReply* reply = (redisReply*)(redisCommand(context, "SET %s ", key));
    if (reply != NULL &&
        reply->str != NULL &&
        reply->type == REDIS_REPLY_STRING)) {
        rc = 0;
		memcpy(value, reply->str, reply->len);
    }

    if (reply != NULL) {
        freeReplyObject(reply);
        reply = NULL;
    }

	return rc;
}

int redisDel(redisContext* context, const char* key) {

}

int redisHSet(redisContext* context, const char* key, const char* field, const char* value) {

}

int redisHGet(redisContext* context, const char* key, const char* field, char* value) {

}

int redisHDel(redisContext* context, const char* key, const char* field) {

}

std::vector<std::string> redisHGetall(redisContext* context, const char* key) {

}

std::unordered_map<std::string, std::string> redisHGetall2(redisContext* context, const char* key) {

}

int redisLPush(redisContext* context, const char* key, const char* value) {

}

int redisLPop(redisContext* context, const char* key, char* value) {

}

int redisRPush(redisContext* context, const char* key, const char* value) {

}

int redisRPop(redisContext* context, const char* key, char* value) {

}

int redisLRem(redisContext* context, const char* key, int top, const char* value) {

}

std::vector<std::string> redisLRange(redisContext* context, const char* key, int beg, int end) {

}

Redis::Redis(struct redisAsyncContext* ctx, struct event_base* base) {
    context_ = ctx;
    base_ = base;
}

Redis::~Redis() {

}

void Redis::updateTimer(struct event* timer, int sec) {
    if (timer != NULL) {
        struct timeval tv = { 0, 0 };
        tv.tv_sec = sec;
        event_add(timer, &tv);
    }
}

void Redis::connectCallback(const struct redisAsyncContext* ctx, int status) {
    Redis* redisConn = static_cast<Redis*>(ctx->data);
    if (event_initialized(&redisConn->check_)) {
        event_del(&redisConn->check_);
    }

    if (status == REDIS_OK) {
        redisConn->status_ = 1;
        // if the event flags == -1 the event must be a timer
         event_assign(&redisConn->check_, redisConn->base_, -1, 0, Redis::checkHealthCallback, redisConn);
    } else {
        redisConn->status_ = 0;
         event_assign(&redisConn->check_, redisConn->base_, -1, 0, Redis::reConnectCallback, redisConn);
    }

    updateTimer(&redisConn->check_, 10);
}

void Redis::disConnectCallback(const struct redisAsyncContext* ctx, int status) {
    Redis* redisConn = static_cast<Redis*>(ctx->data);
    redisConn->status_ = 0;
    if (event_initialized(&redisConn->check_)) {
        event_del(&redisConn->check_);
    }

    event_assign(&redisConn->check_, redisConn->base_, -1, 0, Redis::reConnectCallback, redisConn);
    updateTimer(&redisConn->check_, 10);
}

void Redis::reConnectCallback(int fd, short events, void* arg) {
    Redis* redisConn = static_cast<Redis*>(arg);

    redisConn->context_ = redisAsyncConnect(redisAddr.c_str(), redisPort);
    redisConn->context_->data = redisConn;
    redisLibeventAttach(redisConn->context_, redisConn->base_);
    redisAsyncSetConnectCallback(redisConn->context_, Redis::connectCallback);
    redisAsyncSetDisconnectCallback(redisConn->context_, Redis::disConnectCallback);
}

void Redis::checkHealthCallback(int fd, short events, void* arg) {
    Redis* redisConn = static_cast<Redis*>(arg);
    if (redisConn->status_ == 1) {
        redisAsyncCommand(redisConn->context_, Redis::commandCallback, redisConn, CHECK_HEALTH_CMD);
    }
}

void Redis::commandCallback(struct redisAsyncContext* ctx, void* reply, void* arg) {
    redisReply* rc = static_cast<redisReply*>(reply);
    if (rc == NULL || rc->str == NULL || rc->len == 0) {
        redisAsyncDisconnect(ctx);
        return;
    }

    LOG(INFO) << "redis client run check health success...";

    Redis* redisConn = static_cast<Redis*>(arg);
    updateTimer(&redisConn->check_, 10);
}

void Redis::doCommandCallback(struct redisAsyncContext* ctx, void* reply, void* arg) {
    redisReply* rc = static_cast<redisReply*>(reply);
    if (rc == NULL || rc->type == REDIS_REPLY_ERROR) {
        LOG(ERROR) << "doCommandCallback failed";
    }
}



/*mysql*/
/*
char value = 1;
mysql_options(mysql, MYSQL_OPT_RECONNECT, &value);
*/
MySQL::MySQL(struct event_base* base, const char* host, const char* user, 
            const char* passwd, const char* database, int port) {
    mysql_ = NULL;
    res = NULL;
//    events_ = EV_PERSIST | EV_ET;

//    base_ = base;
    host_ = host;
    user_ = user;
    passwd_ = passwd;
    database_ = database;
    port_ = port;

    MySQLConnect();

    LOG(INFO) << "mysql fd: " << mysql_->net.fd;
/*
    eventMysql_ = event_new(base_, mysql_->net.fd, events_, mysqlcb, this);
    if (eventMysql_ == NULL) {
        exit(-1);
    }

    if (event_add(eventMysql_, NULL) < 0) {
        LOG(ERROR) << "event_add failed";
        exit(-1);
    }
*/
    /*in this not add the timeout event*/
/*  
    eventTimeout_ = event_new(base_, -1, EV_PERSIST, timeoutcb, this);
    if (eventTimeout_ == NULL) {
        exit(-1);
    }
*/
}

MySQL::~MySQL() {
/*
    if (eventMysql_ != NULL) {
        event_safe_free(eventMysql_, event_free);
    }

    if (eventTimeout_ != NULL) {
        event_safe_free(eventTimeout_, event_free);
    }
*/
}

int MySQL::MySQLConnect() {
    int rc = 0;
    mysql_ = mysql_init(mysql_);
    
    if (!mysql_real_connect(mysql_, host_, user_, 
        passwd_, database_, port_, NULL, 0)) {
        LOG(ERROR) << "mysql_reql_connect met error: " << mysql_error(mysql_);
        rc = -1;
    }

    return rc;
}
/*
void MySQL::mysqlcb(int fd, short events, void* arg) {
    LOG(INFO) << "mysql cb";
    MySQL* mysql = static_cast<MySQL*>(arg);
    if (mysql == NULL || mysql_ping(mysql->mysql_) == 0) {
        return;
    }

    LOG(INFO) << "mysql cb now";

    struct timeval tv;
    evutil_timerclear(&tv);
    tv.tv_sec = 5;

    event_add(mysql->eventTimeout_, &tv);
    event_del(mysql->eventMysql_);
}
*/
/*
void MySQL::timeoutcb(int fd, short events, void* arg) {
    MySQL* mysql = static_cast<MySQL*>(arg);
    if (mysql == NULL) {
        return;
    }

    mysql->MySQLConnect();
    event_assign(mysql->eventMysql_, mysql->base_, mysql->mysql_->net.fd, mysql->events_, mysqlcb, arg);    
    if (mysql->eventMysql_ == NULL || event_add(mysql->eventMysql_, NULL) < 0) {
        return;
    }

    //in this don't free event timeout
    event_del(mysql->eventTimeout_);
}
*/

MySQLStatement* MySQL::prepare(const char* sql) {
    return new(std::nothrow) MySQLStatement(mysql_, sql);
}

int MySQL::query(std::string stmt) {
    return 0;
}

char** MySQL::fetch() {
    return NULL;
}

void MySQL::resultFree() {
    return;
}

MySQLStatement::MySQLStatement(MYSQL* mysql, const char* sql) {
    error_ = false;
    stmt_ = mysql_stmt_init(mysql);
    int len = strlen(sql);

    if (mysql_stmt_prepare(stmt_, sql, len)) {
        errorMessage_ = mysql_error(mysql_);
        error_ = true;
        return;
    } 

    args_ = NULL;
    int count = mysql_stmt_param_count(stmt_);

    LOG(INFO) << "mysql stmt param count: " << count;

    if (count > 0) {
        args_ = new(std::nothrow) MYSQL_BIND[count];
        if (args_ == NULL) {
            LOG(ERROR) << "new args_ is nil";
            exit(-1);
        }
    }
}

MySQLStatement::~MySQLStatement() {
    event_safe_free(stmt_, mysql_stmt_free_result);
    delete[] args_;
    args_ = NULL;
}

int MySQLStatement::bind(int pos, void* value, int length, enum_field_types type) {
    if (error_) {
        return -1;
    }

    MYSQL_BIND bind;
    memset(&bind, 0, sizeof(MYSQL_BIND));

    bind.buffer_type = type;
    bind.buffer = static_cast<char*>(value);
    bind.buffer_length = length;
    bind.is_null = 0;

    values_.push_back(bind);

    return 0;
}

int MySQLStatement::params(int pos, void* value, enum_field_types type, my_bool flag) {
    if (error_) {
        return -1;
    }

    memset(&args_[pos], 0, sizeof(MYSQL_BIND));

    if (type == MYSQL_TYPE_NULL) {
        args_[pos].buffer_type = type;
        args_[pos].is_null_value = 1;
    } else {
        args_[pos].buffer_type = type;
        args_[pos].buffer = static_cast<char*>(value);
        args_[pos].is_unsigned = flag;
    }

    return 0;
}

int MySQLStatement::execute() {
    if (error_) {
        return -1;
    }

    if (args_ != NULL) {
        if (mysql_stmt_bind_param(stmt_, args_)) {
            goto error;
        }
    }

    if (mysql_stmt_bind_result(stmt_, &values_[0])) {
        goto error;
    }

    if (mysql_stmt_execute(stmt_)) {
        goto error;
    }

    return 0;

error:
    errorMessage_ = mysql_stmt_error(stmt_);
    error_ = true;
    return -1;
}

int MySQLStatement::fetch() {
    if (error_) {
        return -1;
    }

    int rc = mysql_stmt_fetch(stmt_);
    if (rc == MYSQL_NO_DATA) {
        errorMessage_ = mysql_stmt_error(stmt_);
        error_ = true;
        return -1;
    }

    return 0;
}

int MySQLStatement::allFetch() {
    return 0;
}

int MySQLStatement::columnFetch() {
    return 0;
}

int MySQLStatement::rows() {
    return mysql_affected_rows(mysql_);
}

int MySQLStatement::cols() {
    return 0;
}

}/*end namespace rpc*/

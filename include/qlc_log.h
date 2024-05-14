#ifndef __QLC_LOG_H
#define __QLC_LOG_H

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <time.h>
#include "util.h"
#include "singleton.h"
#include "thread.h"
#define QLC_LOG_LEVEL(logger, level) \
    if(logger->getLevel() <= level) \
        qlc::LogEventWrap(qlc::LogEvent::ptr(new qlc::LogEvent(logger, level, \
                        __FILE__, __LINE__, 0, qlc::GetThreadId(),\
                qlc::GetFiberId(), time(0),qlc::Thread::GetName()))).getSS()
#define QLC_LOG_DEBUG(logger) QLC_LOG_LEVEL(logger, qlc::LogLevel::DEBUG)
#define QLC_LOG_INFO(logger) QLC_LOG_LEVEL(logger, qlc::LogLevel::INFO)
#define QLC_LOG_WARN(logger) QLC_LOG_LEVEL(logger, qlc::LogLevel::WARN)
#define QLC_LOG_ERROR(logger) QLC_LOG_LEVEL(logger, qlc::LogLevel::ERROR)
#define QLC_LOG_FATAL(logger) QLC_LOG_LEVEL(logger, qlc::LogLevel::FATAL)
// 这里解释一下为什么要用LogWarp 因为单纯的LogEvent无法使用流式调用了，所以用析构的方式将缓存的字符串输出
// 这里其实要注意：匿名对象的析构时机和具名对象的析构时机是不一样的
// 匿名对象：执行完后立即析构
// 具名对象(栈内)：超出作用域后析构
// 具名对象(堆内)：手动释放后析构

//下面是format类型的格式打印实现

#define QLC_LOG_FMT_LEVEL(logger, level, fmt, ...) \
    if(logger->getLevel() <= level) \
        qlc::LogEventWrap(qlc::LogEvent::ptr(new qlc::LogEvent(logger, level, \
                        __FILE__, __LINE__, 0, qlc::GetThreadId(),\
                qlc::GetFiberId(), time(0),qlc::Thread::GetName()))).getEvent()->format(fmt, ##__VA_ARGS__)

#define QLC_LOG_FMT_DEBUG(logger, fmt, ...) QLC_LOG_FMT_LEVEL(logger, qlc::LogLevel::DEBUG, fmt, ##__VA_ARGS__)
#define QLC_LOG_FMT_INFO(logger, fmt, ...)  QLC_LOG_FMT_LEVEL(logger, qlc::LogLevel::INFO, fmt, ##__VA_ARGS__)
#define QLC_LOG_FMT_WARN(logger, fmt, ...)  QLC_LOG_FMT_LEVEL(logger, qlc::LogLevel::WARN, fmt, ##__VA_ARGS__)
#define QLC_LOG_FMT_ERROR(logger, fmt, ...) QLC_LOG_FMT_LEVEL(logger, qlc::LogLevel::ERROR, fmt, ##__VA_ARGS__)
#define QLC_LOG_FMT_FATAL(logger, fmt, ...) QLC_LOG_FMT_LEVEL(logger, qlc::LogLevel::FATAL, fmt, ##__VA_ARGS__)

#define QLC_LOG_ROOT()  qlc::Loggermgr::GetInstanceX()->getRoot()
#define QLC_LOG_NAME(name) qlc::Loggermgr::GetInstanceX()->getLogger(name)
namespace qlc{
//日志事件
class Logger;
class LogLevel{
public:
    enum Level{
        UNKNOW=0,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL
    };
    static const char* ToString(LogLevel::Level level);
    static LogLevel::Level FromString(const std::string& str);
};
class LogEvent{
public:

    typedef std::shared_ptr<LogEvent> ptr;
    LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level,const char* file,int32_t line,uint32_t elapse,uint32_t threadId,uint32_t fiberId,uint64_t time,const std::string& thread_name);
    const char *getFile() const {return m_file;}
    int32_t getLine() const {return m_line;}
    uint32_t getElapse() const {return m_elapse;}
    uint32_t getThreadId() const {return m_threadId;}
    uint32_t getFiberId() const {return m_fiberId;}
    uint64_t getTime() const {return m_time;}
    const std::string& getThreadName() const { return m_threadName;}
    const std::string getContent() const {return m_ss.str();}
    std::stringstream& getSS() {return m_ss;}
    LogLevel::Level getLevel() const {return m_level;}
    void format(const char* fmt, ...);
    void format(const char* fmt, va_list al);//格式化写入日志内容
    std::shared_ptr<Logger> getLogger() const {return m_logger;}
private:
    const char* m_file=nullptr; //文件名
    int32_t m_line=0;//第几行
    uint32_t m_elapse=0;//程序启动开始到现在的ms数
    uint32_t m_threadId=0;//线程ID
    uint32_t m_fiberId=0;//协程ID
    uint64_t m_time;//时间戳
    std::stringstream m_ss;//日志内容
    std::shared_ptr<Logger> m_logger;
    LogLevel::Level m_level;
    std::string m_threadName;

};
//主要是为了放event 然后析构  为什么不直接用event的析构 因为那个是智能指针 自己析构了
//主要是利用匿名函数直接析构这一特点 
class LogEventWrap{
public:
    LogEventWrap(LogEvent::ptr e);
    ~LogEventWrap();
    LogEvent::ptr getEvent() const { return m_event;}
    std::stringstream& getSS();
private:
    LogEvent::ptr m_event;


};



class LogFormatter{
public:
    LogFormatter(const std::string& pattern);
    //%t %m %n %thread_id
    typedef std::shared_ptr<LogFormatter> ptr;
    std::string format(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event);//将数据格式化为想要的数据 
    std::ostream& format(std::ostream& ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);
    bool isError() const {return m_error;}
    std::string getPattern() const {return m_pattern;}
public:
    class FormatItem{ //基类 然后根据format的格式解析出item的信息
    public:
        // FormatItem(const std::string &fmt=""){

        // }
        typedef std::shared_ptr<FormatItem> ptr;
        virtual ~FormatItem(){}
        virtual void format(std::shared_ptr<Logger> logger,std::ostream &os ,LogLevel::Level level,LogEvent::ptr event)=0;
    };
    void init();
private:
    bool m_error=false;//格式是否错误 如果错误的话后面报错
    std::string m_pattern;
    std::vector<FormatItem::ptr> m_items;
};  
//日志输出器 
class LogAppender{
friend class Logger;
public:
    typedef Spinlock MutexType;

    typedef std::shared_ptr<LogAppender> ptr;
    virtual std::string toYamlString()=0;
    virtual ~LogAppender(){}//删除在哪个地方的日志是变化的
    virtual void Log(std::shared_ptr<Logger> logger,LogLevel::Level level, const LogEvent::ptr event)=0;//纯虚函数
    void setFormatter(LogFormatter::ptr val);
    LogLevel::Level getLevel() const {return m_level;}
    void setLevel(LogLevel::Level level) {m_level=level;}
    LogFormatter::ptr getFormatter();
    
protected:
    bool hasFormatter =false;//appender是否有自己的formatter 如果没有的话才把logger的formatter传给他
    LogLevel::Level m_level=LogLevel::DEBUG;//要初始化 
    MutexType m_mutex;
    //append应该还要定义输出什么样子的格式
    LogFormatter::ptr m_formatter;
};    

 
//日志器
class Logger: public std::enable_shared_from_this<Logger>{
friend class LoggerManager;
public:
    // int enable_shared_from_this() { }
    typedef std::shared_ptr<Logger> ptr;
    typedef Spinlock MutexType;
    std::string toYamlString();
    Logger(const std::string &name="root");
    void Log(LogLevel::Level level, const LogEvent::ptr event);
    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);
    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    LogLevel::Level getLevel() const {return m_level;}
    void setFormatter(LogFormatter::ptr val);
    LogFormatter::ptr getFormatter(); 
    void setLevel(LogLevel::Level level){m_level =level;}
    const std::string getName() const {return m_name;}
    void setFormatter(const std::string& val);
    void clearAppenders();
private: 
    std::string m_name;
    LogLevel::Level m_level; //满足日志级别的才会输出到日志
    std::list<LogAppender::ptr> m_appenders;
    MutexType m_mutex;
    Logger::ptr m_root; //看看logger是否有自带appender 如果忘记了添加appender 就把mroot的appender发给他 并且每一个logger都有一个相同的root
    LogFormatter::ptr m_formater; //logger自带一个formater  appender如果没有的话就用自带的

};  
//输出到控制台的
class stdoutAppender: public LogAppender{
    //定义只能指针
    std::string toYamlString() override;
    typedef std::shared_ptr<stdoutAppender> ptr;
    void Log(std::shared_ptr<Logger> logger,LogLevel::Level level, const LogEvent::ptr event) override;//override只是用来描述确实是继承得到的


};
class fileAppender:public LogAppender{
public:
    std::string toYamlString() override;
    typedef std::shared_ptr<fileAppender> ptr;
    fileAppender(const std::string& filename);
    void Log(std::shared_ptr<Logger> logger,LogLevel::Level level, const LogEvent::ptr event) override;//override只是用来描述确实是继承得到的
    bool reopen();//文件打开了的话 先关闭然后重新打开
private:
    std::string m_filename;
    std::ofstream m_filestream;//使用流的方式写入文件
    uint64_t m_lastTime = 0;//防止写到一半突然把文件删除了
};
//日志管理器
class LoggerManager{
public:
    LoggerManager();
    typedef Spinlock MutexType;
    std::string toYamlString();
    Logger::ptr getLogger(const std::string& name);
    //返回主日志器
    Logger::ptr getRoot() const { return m_root;}
    void init();
private:
    std::map<std::string,Logger::ptr> m_loggers;
    Logger::ptr m_root;
    MutexType m_mutex;
};
typedef qlc::Singleton<LoggerManager> Loggermgr;//logger管理类只有一个 所以需要单例模式
}


#endif
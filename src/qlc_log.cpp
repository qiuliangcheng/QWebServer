#include "qlc_log.h"
#include <functional>
#include <string.h>
#include <stdarg.h>
#include "config.h"

//首先实现日志器
namespace qlc{
    LogEvent::LogEvent(std::shared_ptr<Logger> logger,LogLevel::Level level,const char* file,int32_t line,uint32_t elapse,uint32_t threadId,uint32_t fiberId,uint64_t time):
        m_file(file),m_line(line),m_elapse(elapse),m_threadId(threadId),m_fiberId(fiberId),m_time(time),m_logger(logger),m_level(level){

    }

    void LogEvent::format(const char *fmt, ...)
    {
            va_list al;
            va_start(al, fmt);
            format(fmt, al);
            va_end(al);
    }

    void LogEvent::format(const char *fmt, va_list al)
    {
            char* buf = nullptr;
            int len = vasprintf(&buf, fmt, al);
            if(len != -1) {
                m_ss << std::string(buf, len);
                free(buf);
            }
    }

    std::string Logger::toYamlString()
    {
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        node["name"] = m_name;
        if(m_level != LogLevel::UNKNOW) {
            node["level"] = LogLevel::ToString(m_level);
        }
        if(m_formater){
            node["formatter"] = m_formater->getPattern();
        }
        if(!m_appenders.empty()){
            for(auto& i : m_appenders) {
            node["appenders"].push_back(YAML::Load(i->toYamlString()));
        }
        }   
        std::stringstream ss;
        ss << node;
        return ss.str();     
    }

    Logger::Logger(const std::string &name) : m_name(name), m_level(LogLevel::DEBUG)
    {
        m_formater.reset(new LogFormatter("%d %f [%p] %l %m %n"));//首先为自己设置一个loggerformatter 防止后面用到append没有设置
    }                                                             //
    void Logger::Log(LogLevel::Level level, const LogEvent::ptr event){
         std::shared_ptr<Logger> sharedThis = shared_from_this();
         MutexType::Lock lock(m_mutex);
        if(level>=m_level){
        if(!m_appenders.empty()){
            for(auto i:m_appenders){
                i->Log(sharedThis,level,event);
            }
        }else{
            m_root->Log(level,event);
        }

        }
    }
    void Logger::debug(LogEvent::ptr event){
        Log(LogLevel::DEBUG,event);

    }
    void Logger::info(LogEvent::ptr event){
        Log(LogLevel::INFO,event);

    }
    void Logger::warn(LogEvent::ptr event){
        Log(LogLevel::WARN,event);

    }
    void Logger::error(LogEvent::ptr event){
        Log(LogLevel::ERROR,event);

    }
    void Logger::fatal(LogEvent::ptr event){
        Log(LogLevel::FATAL,event);

    }
    void Logger::addAppender(LogAppender::ptr appender){
        MutexType::Lock lock(m_mutex);
        if(!appender->getFormatter()){//如果要加的appender没有formatter
        //把自己的格式加给他
            MutexType::Lock ll(appender->m_mutex);
            appender->m_formatter=m_formater;
        }
        m_appenders.push_back(appender);

    }
    void Logger::delAppender(LogAppender::ptr appender){
        MutexType::Lock lock(m_mutex);
        for(auto it=m_appenders.begin();it!=m_appenders.end();it++)
        {
            if(*it==appender){
                m_appenders.erase(it);
                break;
            }
        }
    }
    void Logger::setFormatter(LogFormatter::ptr val)
    {
        MutexType::Lock lock(m_mutex);
        m_formater = val;
        //如果appender下面没有自定义的格式 则全部改成Logger的格式
        for(auto &i :m_appenders)
        {
            MutexType::Lock ll(i->m_mutex);
            if(!i->hasFormatter){
                i->m_formatter=m_formater;
            }
        }
    }
    LogFormatter::ptr Logger::getFormatter()
    {
        MutexType::Lock lock(m_mutex);
        return m_formater;
    }
    void Logger::clearAppenders()
    {
        MutexType::Lock lock(m_mutex);
        m_appenders.clear();
    }
    void Logger::setFormatter(const std::string &val)
    {
        qlc::LogFormatter::ptr formatter(new LogFormatter(val));
        if(formatter->isError()){
            std::cout<<"logger设置的格式错误了"<<std::endl;
            return;
        }
        setFormatter(formatter);
    }
    const char *LogLevel::ToString(LogLevel::Level level)
    {
        switch (level)
        {
#define XX(name) \
    case LogLevel::name: \
        return #name;\
        break;
    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
#undef XX
    default:
        return "UNKNOW";

    }

    return "UNKNOW";
}
LogLevel::Level LogLevel::FromString(const std::string &str)
{
#define XX(level,m_str) \
    if(str==#m_str) \
    {return LogLevel::level;}
    XX(DEBUG, debug);
    XX(INFO, info);
    XX(WARN, warn);
    XX(ERROR, error);
    XX(FATAL, fatal);

    XX(DEBUG, DEBUG);
    XX(INFO, INFO);
    XX(WARN, WARN);
    XX(ERROR, ERROR);
    XX(FATAL, FATAL);
    return LogLevel::UNKNOW;
#undef XX

}
// 输出器
fileAppender::fileAppender(const std::string& filename):m_filename(filename){
    reopen();
}

bool fileAppender::reopen(){
    MutexType::Lock lock(m_mutex);
    if(m_filestream) m_filestream.close();//先关闭
    m_filestream.open(m_filename,std::ios::out | std::ios::binary);
    // std::cout<<!!m_filestream;
    return !!m_filestream;//双感叹号 把非0值 变成1 0值还是不变
}
std::string fileAppender::toYamlString()
{
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    node["type"] = "fileAppender";
    node["file"] = m_filename;
    if(m_level != LogLevel::UNKNOW) {
        node["level"] = LogLevel::ToString(m_level);
    }
    if(hasFormatter && m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}
void fileAppender::Log(std::shared_ptr<Logger> logger, LogLevel::Level level, const LogEvent::ptr event)
{
    if(level >= m_level) {//每隔三秒就重新打开一次 防止你突然删除某个文件
        uint64_t now = event->getTime();
        if(now >= (m_lastTime + 3)) {
            reopen();
            m_lastTime = now;
        }
        MutexType::Lock lock(m_mutex);
        //if(!(m_filestream << m_formatter->format(logger, level, event))) {
        if(!m_formatter->format(m_filestream,logger, level, event)) {
            std::cout << "error" << std::endl;
        }
    }
}
void stdoutAppender::Log(std::shared_ptr<Logger> logger,LogLevel::Level level, const LogEvent::ptr event){
    if(level>=m_level){
        MutexType::Lock lock(m_mutex);
        std::cout<< m_formatter->format(logger,level,event);
    }

}
std::string stdoutAppender::toYamlString()
{
    YAML::Node node;
    node["type"] = "stdoutAppender";
    if(m_level != LogLevel::UNKNOW) {
        node["level"] = LogLevel::ToString(m_level);
    }
    if(hasFormatter && m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

void LogAppender::setFormatter(LogFormatter::ptr val)
{
    MutexType::Lock lock(m_mutex);//这边不能往下走 
    m_formatter = val;
    if(m_formatter) {
        hasFormatter = true;
    } else {
        hasFormatter = false;
    }
}
LogFormatter::ptr LogAppender::getFormatter()
{
    MutexType::Lock lock(m_mutex);
    return m_formatter;
}
// 接下来就是formatter的实现

LogFormatter::LogFormatter(const std::string &pattern):m_pattern(pattern)
{
    init();//初始化
}
// 日志格式器，执行日志格式化，负责日志格式的初始化。
// 解析日志格式，将用户自定义的日志格式，解析为对应的FormatItem。
// 日志格式举例：%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n
// 格式解析：
// %d{%Y-%m-%d %H:%M:%S} : %d 标识输出的是时间 {%Y-%m-%d %H:%M:%S}为时间格式，可选 DateTimeFormatItem
// %T : Tab[\t]            TabFormatItem
// %t : 线程id             ThreadIdFormatItem
// %N : 线程名称           ThreadNameFormatItem
// %F : 协程id             FiberIdFormatItem
// %p : 日志级别           LevelFormatItem       
// %c : 日志名称           NameFormatItem
// %f : 文件名             FilenameFormatItem
// %l : 行号               LineFormatItem
// %m : 日志内容           MessageFormatItem
// %n : 换行符[\r\n]       NewLineFormatItem
class MessageFormatItem : public LogFormatter::FormatItem{
public:
    MessageFormatItem(const std::string &fmt=""){}
    void format(std::shared_ptr<Logger> logger,std::ostream &os ,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getContent();        
    }
};
class LevelFormatItem : public LogFormatter::FormatItem{
public:
LevelFormatItem(const std::string &fmt=""){}
void format(std::shared_ptr<Logger> logger,std::ostream &os ,LogLevel::Level level,LogEvent::ptr event) override{
            os<<LogLevel::ToString(level);        
    }
};
class ElapseFormatItem : public LogFormatter::FormatItem{
public:
ElapseFormatItem(const std::string &fmt=""){}
void format(std::shared_ptr<Logger> logger,std::ostream &os ,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getElapse();         
    }
};
class NameFormatItem : public LogFormatter::FormatItem{
public:
NameFormatItem(const std::string &fmt=""){}
void format(std::shared_ptr<Logger> logger,std::ostream &os ,LogLevel::Level level,LogEvent::ptr event) override{
        os<<logger->getName();         
    }
};
class ThreadIdFormatItem : public LogFormatter::FormatItem{
public:
ThreadIdFormatItem(const std::string &fmt=""){}
void format(std::shared_ptr<Logger> logger,std::ostream &os ,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getThreadId();         
    }
};
class FiberFormatItem : public LogFormatter::FormatItem{
public:
FiberFormatItem(const std::string &fmt=""){}
void format(std::shared_ptr<Logger> logger,std::ostream &os ,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getFiberId();         
    }
};
class DataTimeFormatItem : public LogFormatter::FormatItem{
public:
    DataTimeFormatItem(const std::string &format="%Y-%m-%d %H:%M:%S"):m_format(format){
    // std::cout<<"进来时间这里了"<<std::endl;
    if(m_format.empty()) m_format="%Y-%m-%d %H:%M:%S";
}
    void format(std::shared_ptr<Logger> logger,std::ostream &os ,LogLevel::Level level,LogEvent::ptr event) override{
        struct tm tm;
        time_t time=event->getTime();
        localtime_r(&time,&tm);
        char buf[64];
        strftime(buf,sizeof(buf),m_format.c_str(),&tm);
        os<<buf;         
    }
private:
    std::string m_format;
};
//文件名
class FileNameFormatItem : public LogFormatter::FormatItem{
public:
FileNameFormatItem(const std::string &fmt=""){}
void format(std::shared_ptr<Logger> logger,std::ostream &os ,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getFile();         
    }
};
//行号
class LineFormatItem : public LogFormatter::FormatItem{
public:
LineFormatItem(const std::string &fmt=""){}
void format(std::shared_ptr<Logger> logger,std::ostream &os ,LogLevel::Level level,LogEvent::ptr event) override{
        os<<event->getLine();         
    }
};
//换行
class NewLineFormatItem : public LogFormatter::FormatItem{
public:
NewLineFormatItem(const std::string &fmt=""){}
void format(std::shared_ptr<Logger> logger,std::ostream &os ,LogLevel::Level level,LogEvent::ptr event) override{
        os<<std::endl;         
    }
};
class TabFormatItem : public LogFormatter::FormatItem{
public:
TabFormatItem(const std::string &fmt=""){}
void format(std::shared_ptr<Logger> logger,std::ostream &os ,LogLevel::Level level,LogEvent::ptr event) override{
        os<<'\t';         
    }
};
class StringFormatItem : public LogFormatter::FormatItem{
public:
    StringFormatItem(const std::string &str):m_sting(str){

}
    void format(std::shared_ptr<Logger> logger,std::ostream &os ,LogLevel::Level level,LogEvent::ptr event) override{
        os<<m_sting;         
    }
private:
    std::string m_sting;
};
class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    FiberIdFormatItem(const std::string& str = "") {}
    void format(std::shared_ptr<Logger> logger,std::ostream &os ,LogLevel::Level level,LogEvent::ptr event) override{
        os << event->getFiberId();
    }
};
std::string LogFormatter::format(std::shared_ptr<Logger> logger,LogLevel::Level level,LogEvent::ptr event)
{
    std::stringstream ss;
    for(auto &i : m_items){
        i->format(logger,ss,level,event);
    }
    return ss.str();
}

std::ostream &LogFormatter::format(std::ostream &ofs, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
    for(auto& i : m_items) {
        i->format(logger, ofs,level, event);
    }
    return ofs;
}

void LogFormatter::init()//日志格式定义
{   //str format type 三元组的格式
    // std::cout<<m_pattern<<std::endl;
    std::vector<std::tuple<std::string,std::string,int>> vec;
    std::string nstr;//保存%后面的格式  保存的是比如aaa%d 中的aaa 固定输出
    //在字符串str的末尾添加n个字符c
    // string& append (size_t n, char c);
    // std::cout<<m_pattern<<std::endl;
    for(size_t i=0;i<m_pattern.size();i++){
        if(m_pattern[i]!='%'){
            nstr.append(1,m_pattern[i]);
            continue;
        }
        if((i+1)<m_pattern.size()){
            if(m_pattern[i+1]=='%'){
                nstr.append(1,'%');
                continue;
            }
        }
        //也就是说找个是%号
        int format_status=0;//状态0是 %s  状态1 %s{  状态2是%s{}
        size_t n=i+1;
        size_t format_begin=0;
        std::string str; //保存的是%sss{xx}中的sss
        std::string fmt;
        while( n < m_pattern.size()){
            // if(isspace(m_pattern[n])) break; //是空格符号的话
            if(!format_status&&!isalpha(m_pattern[n])&&m_pattern[n]!='{'&&m_pattern[n]!='}'){
                str=m_pattern.substr(i+1,n-i-1);
                break;
            }
            if( format_status==0){
                if(m_pattern[n]=='{'){
                    str=m_pattern.substr(i+1,n-i-1);
                    format_status=1;
                    format_begin=n;
                    n++;
                    continue;
                }
            }
            else if( format_status==1){
                if(m_pattern[n]=='}'){
                    fmt=m_pattern.substr(format_begin+1,n-format_begin-1);
                    format_status=0;
                    n++;
                    break;
                }
            }
            n++;
            if(n==m_pattern.size()){
                if(str.empty()){
                    str=m_pattern.substr(i+1);
                    // std::cout<<"这个是"<<str<<std::endl;
                }
            }
        }
        if(format_status==0){
            if(!nstr.empty()){
                vec.push_back(std::make_tuple(nstr,"",0));
                nstr.clear();
            }
            
            // str=m_pattern.substr(i+1,n-i-1);
            // std::cout<<str<<"这个是百分号后面的值"<<std::endl;
            vec.push_back(std::make_tuple(str,fmt,1));
            i=n-1;
        }
        else if(format_status==1){
            m_error = true;
            std::cout<<"格式错误"<<m_pattern<<"-"<<m_pattern.substr(i)<<std::endl;
            vec.push_back(std::make_tuple("pattern error",fmt,0));
        }
        // if(format_status==2){
        //     if(!nstr.empty()){
        //         vec.push_back(std::make_tuple(nstr,"",0));
        //         nstr.clear();s
        //     }
        //     vec.push_back(std::make_tuple(str,fmt,1));
        //     i=n-1;
        // }
    }
   if(!nstr.empty()){
        vec.push_back(std::make_tuple(nstr,"",0));
    } 
   static std::map<std::string,std::function<FormatItem::ptr(const std::string &str)>> s_format_item={
#define AA(str,C)\
        {#str,[](const std::string&fmt){return FormatItem::ptr(new C(fmt));}}

        AA(m,MessageFormatItem),
        AA(p,LevelFormatItem),
        AA(r,ElapseFormatItem),
        AA(c,NameFormatItem),
        AA(t,ThreadIdFormatItem),
        AA(n,NewLineFormatItem), 
        AA(d,DataTimeFormatItem),
        AA(f,FileNameFormatItem),
        AA(l,LineFormatItem),
        AA(T,TabFormatItem),
        AA(F,FiberIdFormatItem)
#undef AA
   };
//    for(auto &i:s_format_item){
//         std::cout<<i.first<<std::endl;
//    }
//std::get<> 是 C++ 标准库中的一个函数模板，用于从元组（tuple）或 pair（二元组）中获取指定位置的值。
   for(auto &i:vec){
        if(std::get<2>(i)==0){
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        }else{
            auto it=s_format_item.find(std::get<0>(i));
            if(it==s_format_item.end()){
                m_items.push_back(FormatItem::ptr(new StringFormatItem(std::string("error format") + " " + std::get<0>(i))));
                m_error = true;
            }
            else{
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }

   }

}
LogEventWrap::LogEventWrap(LogEvent::ptr e):m_event(e)
{

}
LogEventWrap::~LogEventWrap() {
    m_event->getLogger()->Log(m_event->getLevel(), m_event);
}
std::stringstream& LogEventWrap::getSS() {
    return m_event->getSS();
}
LoggerManager::LoggerManager()
{
    m_root.reset(new Logger);
    m_root->addAppender(LogAppender::ptr(new stdoutAppender));
    m_loggers[m_root->getName()] = m_root;
    init();
}
Logger::ptr LoggerManager::getLogger(const std::string &name)
{
    MutexType::Lock lock(m_mutex);
    auto it = m_loggers.find(name);
    if(it != m_loggers.end()) {
        return it->second;
    }
    Logger::ptr logger(new Logger(name));
    logger->m_root = m_root;
    m_loggers[name] = logger;
    return logger;
}
struct LogAppenderDefine {
    int type =0;
    LogLevel::Level level=LogLevel::UNKNOW;
    std::string formatter;
    std::string file;
    bool operator==(const LogAppenderDefine& oth) const {
        return type == oth.type
            && level == oth.level
            && formatter == oth.formatter
            && file == oth.file;
    }

};
struct LogDefine {
    std::string name;
    LogLevel::Level level = LogLevel::UNKNOW;
    std::string formatter;
    std::vector<LogAppenderDefine> appenders;//一个logger可以有多个appender

    bool operator==(const LogDefine& oth) const {
        return name == oth.name
            && level == oth.level
            && formatter == oth.formatter
            && appenders == appenders;
    }

    bool operator<(const LogDefine& oth) const {
        return name < oth.name;
    }

    bool isValid() const {
        return !name.empty();
    }
};
template<>
class LexicalCast<std::string, LogDefine> {
public:
    LogDefine operator()(const std::string& v) {
        
        YAML::Node n = YAML::Load(v);
        LogDefine ld;
        if(!n["name"].IsDefined()) {
            std::cout << "名字没有定义 " << n
                      << std::endl;
        }
        ld.name = n["name"].as<std::string>();
        ld.level = LogLevel::FromString(n["level"].IsDefined() ? n["level"].as<std::string>() : "");
        if(n["formatter"].IsDefined()) {
            ld.formatter = n["formatter"].as<std::string>();
        }

        if(n["appenders"].IsDefined()) {
            for(size_t x = 0; x < n["appenders"].size(); ++x) {
                auto a = n["appenders"][x];
                if(!a["type"].IsDefined()) {
                    std::cout << "输出类型错误 " << a
                              << std::endl;
                    continue;
                }
                std::string type = a["type"].as<std::string>();
                LogAppenderDefine lad;
                if(type == "fileAppender") {
                    lad.type = 1;
                    if(!a["file"].IsDefined()) {
                        std::cout << "文件路径没有定义" << a
                              << std::endl;
                        continue;
                    }
                    lad.file = a["file"].as<std::string>();
                    if(a["formatter"].IsDefined()) {
                        lad.formatter = a["formatter"].as<std::string>();
                    }
                } else if(type == "stdoutAppender") {
                    lad.type = 2;
                    if(a["formatter"].IsDefined()) {
                        lad.formatter = a["formatter"].as<std::string>();
                    }
                } else {
                    std::cout << "你定义的输出地方暂时不正确" << a
                              << std::endl;
                    continue;
                }

                ld.appenders.push_back(lad);
            }
        }
        return ld;
    }
};



template<>
class LexicalCast<LogDefine, std::string> {
public:
    std::string operator()(const LogDefine& i) {
        
        YAML::Node n;
        n["name"] = i.name;
        if(i.level != LogLevel::UNKNOW) {
            n["level"] = LogLevel::ToString(i.level);
        }
        if(!i.formatter.empty()) {
            n["formatter"] = i.formatter;
        }

        for(auto& a : i.appenders) {
            YAML::Node na;
            if(a.type == 1) {
                na["type"] = "fileAppender";
                na["file"] = a.file;
            } else if(a.type == 2) {
                na["type"] = "stdoutAppender";
            }
            if(a.level != LogLevel::UNKNOW) {
                na["level"] = LogLevel::ToString(a.level);
            }

            if(!a.formatter.empty()) {
                na["formatter"] = a.formatter;
            }

            n["appenders"].push_back(na);
        }
        std::stringstream ss;
        ss << n;
        return ss.str();
    }
};
qlc::ConfigVar<std::set<LogDefine>>::ptr log_value_config=qlc::Config::Lookup("logs", std::set<LogDefine>(), "logs config");

//接下来就是增加监听函数了 发现变化就创建一个新的logger
//全局的初始化会在前面一些
//通过监听函数 来生成新的Logger 并且使用struct来替代class的信息
struct LogIniter {
    LogIniter(){
        log_value_config->addListener([](const std::set<LogDefine>& old_value,
                    const std::set<LogDefine>& new_value){
                        for(auto &i : new_value){
                            auto it=old_value.find(i);
                            Logger::ptr logger;
                            if(it==old_value.end()){
                                //新增
                                logger=QLC_LOG_NAME(i.name);
                            }else{
                                //内容不一样
                                continue;
                            }
                            logger->setLevel(i.level);
                            if(!i.formatter.empty()) {
                                logger->setFormatter(i.formatter);
                            }
                            logger->clearAppenders();
                            for(auto& a : i.appenders){
                                LogAppender::ptr lap;
                                if(a.type == 1) {
                                    lap.reset(new fileAppender(a.file));
                                } else if(a.type == 2) {
                                        lap.reset(new stdoutAppender);
                                } 
                                lap->setLevel(a.level);
                                if(!a.formatter.empty()) {
                                    LogFormatter::ptr fmt(new LogFormatter(a.formatter));
                                    if(!fmt->isError()) {
                                        lap->setFormatter(fmt);
                                    }else{
                                        std::cout<<"你 定义的输出地方的格式是有问题的"<<std::endl;
                                    }

                                }
                                logger->addAppender(lap);
                            }
                        }
                    for(auto& i : old_value) {
                        auto it = new_value.find(i);
                        if(it == new_value.end()) {
                            //删除logger
                            auto logger = QLC_LOG_NAME(i.name);
                            logger->setLevel((LogLevel::Level)100);
                            logger->clearAppenders();
                         }
                    }
                });
    }
};
static LogIniter __log_init;//没懂这个是什么意思 
void LoggerManager::init()
{
}

std::string LoggerManager::toYamlString()
{
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    for(auto& i : m_loggers) {
        node.push_back(YAML::Load(i.second->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}
}
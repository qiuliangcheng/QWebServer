#ifndef __QLC_CONFIG_H__
#define __QLC_CONFIG_H__
#include <memory>
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "qlc_log.h"
#include <map>
#include <vector>
#include <list>
#include <unordered_set>
#include <unordered_map>
#include <yaml-cpp/yaml.h>
#include <functional>
namespace qlc{
class ConfigVarBase {
public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string& name, const std::string& description="")
        :m_name(name)
        ,m_description(description) {
        std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
    }
    virtual ~ConfigVarBase() {}
    const std::string& getName() const { return m_name;}
    const std::string& getDescription() const { return m_description;}

    virtual std::string toString() = 0;

    virtual bool fromString(const std::string& val) = 0;
    virtual std::string getTypeName() const = 0;
protected:
    /// 配置参数的名称
    std::string m_name;
    /// 配置参数的描述
    std::string m_description;
};

//普通类型转换
template<class F,class T>
class LexicalCast{
public:
    T operator()(const F&v){
        return boost::lexical_cast<T>(v);
    }
};
//vector
template<class T>
class LexicalCast<std::string, std::vector<T>>{
public:
    std::vector<T> operator()(const std::string&v){
        YAML::Node node = YAML::Load(v);
        typename std::vector<T> vec;
        std::stringstream ss;
        for(size_t i=0;i<node.size();i++){
            ss.str("");
            ss<<node[i];
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};
template<class T>
class LexicalCast<std::vector<T>, std::string> {
public:
    std::string operator()(const std::vector<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


//list
template<class T>
class LexicalCast<std::string, std::list<T>>{
public:
    std::list<T> operator()(const std::string&v){
        YAML::Node node = YAML::Load(v);
        typename std::list<T> m_list;
        std::stringstream ss;
        for(size_t i=0;i<node.size();i++){
            ss.str("");
            ss<<node[i];
            m_list.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return m_list;
    }
};
template<class T>
class LexicalCast<std::list<T>, std::string> {
public:
    std::string operator()(const std::list<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


//set
template<class T>
class LexicalCast<std::string, std::set<T>>{
public:
    std::set<T> operator()(const std::string&v){
        YAML::Node node = YAML::Load(v);
        typename std::set<T> m_set;
        std::stringstream ss;
        for(size_t i=0;i<node.size();i++){
            ss.str("");
            ss<<node[i];
            m_set.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return m_set;
    }
};
template<class T>
class LexicalCast<std::set<T>, std::string> {
public:
    std::string operator()(const std::set<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


//unorderd_set
template<class T>
class LexicalCast<std::string, std::unordered_set<T> > {
public:
    std::unordered_set<T> operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        typename std::unordered_set<T> vec;
        std::stringstream ss;
        for(size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            vec.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

template<class T>
class LexicalCast<std::unordered_set<T>, std::string> {
public:
    std::string operator()(const std::unordered_set<T>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for(auto& i : v) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

//map
template<class T>
class LexicalCast<std::string, std::map<std::string, T> >{
public:
    std::map<std::string, T> operator()(const std::string& v){
        YAML::Node node =YAML::Load(v);
        typename std::map<std::string, T> m_map;
        std::stringstream ss;
        for(auto it=node.begin();it!=node.end();it++){
            ss.str("");
            ss<<it->second;
            m_map.insert(std::make_pair(it->first.Scalar(),LexicalCast<std::string,T>()(ss.str())));

        }
        return m_map;
    }
};
template<class T>
class LexicalCast<std::map<std::string, T>, std::string> {
public:
    std::string operator()(const std::map<std::string, T>& v) {
        YAML::Node node(YAML::NodeType::Map);
        for(auto& i : v) {
            node[i.first] = YAML::Load(LexicalCast<T, std::string>()(i.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


//任意类型 可以把值转为string 或者反序列化
template<class T,class FromStr=LexicalCast<std::string,T>, class ToStr=LexicalCast<T,std::string>>
class ConfigVar : public ConfigVarBase {
public:
    typedef RWMutex MutexType;
    typedef std::shared_ptr<ConfigVar> ptr;
    typedef std::function<void (const T& old_value, const T& new_value)> on_change_cb;
    ConfigVar(const std::string& name
            ,const T& default_value
            ,const std::string& description = "")
        :ConfigVarBase(name, description)
        ,m_val(default_value) {
    }

    /**
     * @brief 将参数值转换成YAML String
     * @exception 当转换失败抛出异常
     */
    std::string toString() override {
        try {
            MutexType::ReadLock lock(m_mutex);
            return ToStr()(m_val); 
        } catch (std::exception& e) {
            QLC_LOG_ERROR(QLC_LOG_ROOT()) << "ConfigVar::toString exception "
                << e.what() << " convert: " << typeid(m_val).name() << " to string"
                << " name=" << m_name;
        }
        return "";
    }

    /**
     * @brief 从YAML String 转成参数的值
     * @exception 当转换失败抛出异常
     */
    bool fromString(const std::string& val) override {
        try {
           setValue(FromStr()(val));
        } catch (std::exception& e) {
            QLC_LOG_ERROR(QLC_LOG_ROOT()) << "ConfigVar::fromString exception "
                << e.what() << " convert: string to " << typeid(m_val).name();

        }
        return false;
    }
    std::string getTypeName() const override {
        return typeid(T).name();
    }
    const T getValue() {
        MutexType::ReadLock lock(m_mutex);
        return m_val;
    }
    void setValue(const T &v)
    {   
        {
            
            MutexType::ReadLock lock(m_mutex);
            if(m_val == v){
                
                return;
            }
            for(auto &i :m_cbs){
                i.second(m_val,v);
            }
        }
        
        MutexType::WriteLock ll(m_mutex);
        m_val = v;
    }

    uint64_t addListener(on_change_cb cb) {
        MutexType::WriteLock ll(m_mutex);
        static uint64_t s_fun_id = 0;
        ++s_fun_id;
        m_cbs[s_fun_id] = cb;
        return s_fun_id;
    }
    void delListener(uint64_t key) {
        MutexType::WriteLock ll(m_mutex);
        m_cbs.erase(key);
    }
    on_change_cb getListener(uint64_t key) {
        MutexType::ReadLock lock(m_mutex);
        auto it = m_cbs.find(key);
        return it == m_cbs.end() ? nullptr : it->second;
    }
    void clearListener() {
        m_cbs.clear();
    }

private:
    MutexType m_mutex;
    T m_val;
    std::map<uint64_t, on_change_cb> m_cbs;
};

class Config {
public:
    typedef RWMutex MutexType;
    typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigVarMap;
    /*
     * @brief 获取/创建对应参数名的配置参数
     * @param[in] name 配置参数名称
     * @param[in] default_value 参数默认值
     * @param[in] description 参数描述
     * @details 获取参数名为name的配置参数,如果存在直接返回
     *          如果不存在,创建参数配置并用default_value赋值
     * @return 返回对应的配置参数,如果参数名存在但是类型不匹配则返回nullptr
     * @exception 如果参数名包含非法字符[^0-9a-z_.] 抛出异常 std::invalid_argument
     */
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name,
            const T& default_value, const std::string& description = "") {
        auto it = GetDatas().find(name);
        MutexType::WriteLock lock(GetMutex());
        if(it != GetDatas().end()) {
            auto tmp = std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
            if(tmp) {
                QLC_LOG_INFO(QLC_LOG_ROOT()) << "Lookup name=" << name << " exists";
                return tmp;
            } else {
                QLC_LOG_ERROR(QLC_LOG_ROOT()) << "Lookup name=" << name << " exists but type not ";
                        // << " real_type=" << it->second->getTypeName()
                        // << " " << it->second->toString();
                return nullptr; 
            }
        }

        if(name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._012345678")
                != std::string::npos) {
            QLC_LOG_ERROR(QLC_LOG_ROOT()) << "Lookup name invalid " << name;
            throw std::invalid_argument(name);
        }

        typename ConfigVar<T>::ptr v(new ConfigVar<T>(name, default_value, description));
        GetDatas()[name] = v;
        return v;
    }

    /**
     * @brief 查找配置参数
     * @param[in] name 配置参数名称
     * @return 返回配置参数名为name的配置参数
     */
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name) {
        MutexType::ReadLock lock(GetMutex());
        auto it = GetDatas().find(name);
        if(it == GetDatas().end()) {
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
    }

    static void LoadFromYaml(const YAML::Node& root);

    /**
     * @brief 加载path文件夹里面的配置文件
     */
    // static void LoadFromConfDir(const std::string& path, bool force = false);

    static ConfigVarBase::ptr LookupBase(const std::string& name);

    /**
     * @brief 遍历配置模块里面所有配置项
     * @param[in] cb 配置项回调函数
     */
    static void Visit(std::function<void(ConfigVarBase::ptr)> cb);
private:
    /**
     * @brief 返回所有的配置项
     */
    //因为静态成员初始化没有严格的顺序 所以得用这种方式 防止 我的ConfigVar的lookup函数初始化的时候但是我的map还没初始化成功
    static ConfigVarMap& GetDatas() {
        static ConfigVarMap s_datas;
        return s_datas;
    }
    static MutexType& GetMutex(){
        static MutexType m_mutex;
        return m_mutex;
    }
    

};


}


#endif
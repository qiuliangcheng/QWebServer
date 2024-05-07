#include <iostream>
#include <pthread.h>
#include "qlc_log.h"
#include <yaml-cpp/yaml.h>
#include "config.h"
qlc::ConfigVar<int>::ptr int_value_config = qlc::Config::Lookup("system.my.port",(int)8080,"system.port");
qlc::ConfigVar<float>::ptr float_value_config = qlc::Config::Lookup("system.value",(float)1222,"system.value");
qlc::ConfigVar<std::vector<int>>::ptr vec_value_config = qlc::Config::Lookup("system.vec_value",std::vector<int>{1,2},"system.VECTOR");
qlc::ConfigVar<std::map<std::string,int>>::ptr map_value_config = qlc::Config::Lookup("system.map_value",std::map<std::string,int>{{"k1",22}},"system.map");

void print_yaml(const YAML::Node& node, int level) {
    if(node.IsScalar()) {
        QLC_LOG_INFO(QLC_LOG_ROOT()) << std::string(level * 4, ' ')
            << node.Scalar() << " - " << node.Type() << " - " << level;
    } else if(node.IsNull()) {
        QLC_LOG_INFO(QLC_LOG_ROOT()) << std::string(level * 4, ' ')
            << "NULL - " << node.Type() << " - " << level;
    } else if(node.IsMap()) {
        for(auto it = node.begin();
                it != node.end(); ++it) {
            QLC_LOG_INFO(QLC_LOG_ROOT()) << std::string(level * 4, ' ')
                    << it->first << " - " << it->second.Type() << " - " << level;
            print_yaml(it->second, level + 1);
        }
    } else if(node.IsSequence()) {
        for(size_t i = 0; i < node.size(); ++i) {
            QLC_LOG_INFO(QLC_LOG_ROOT()) << std::string(level * 4, ' ')
                << i << " - " << node[i].Type() << " - " << level;
            print_yaml(node[i], level + 1);
        }
    }
}
void test_yaml() {
    YAML::Node root = YAML::LoadFile("../log.yml");
    print_yaml(root, 0);
    //SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << root.Scalar();
}
//自定义类的解析
class Person{
public:
    Person(){}
    std::string name="";
    int age  = 0;
    bool sex = 1;
    std::string toString() const {
        std::stringstream ss;
        ss.str("");
        ss<<"person name = "<<name <<"person age = "<<age<<"person sex= "<<sex;
        return ss.str();
    }  
    bool operator==(const Person& oth) const {
        return name == oth.name
            && age == oth.age
            && sex == oth.sex;
    }
};
namespace qlc {

template<>
class LexicalCast<std::string, Person> {
public:

    Person operator()(const std::string& v) {
        
        YAML::Node node = YAML::Load(v);
        Person p;
        p.name = node["name"].as<std::string>();
        p.age = node["age"].as<int>();
        p.sex = node["sex"].as<bool>();
        return p;
    }
};

template<>
class LexicalCast<Person, std::string> {
public:
    std::string operator()(const Person& p) {
        YAML::Node node;
        node["name"] = p.name;
        node["age"] = p.age;
        node["sex"] = p.sex;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};
}
qlc::ConfigVar<Person>::ptr Person_value_config = qlc::Config::Lookup("class.person",Person(),"class.person");



void test_config() {
    // QLC_LOG_INFO(QLC_LOG_ROOT()) << "before: " << int_value_config->getValue();
    // QLC_LOG_INFO(QLC_LOG_ROOT()) << "before: " << float_value_config->toString();
    #define XX(g_var, name, prefix) \
    { \
        auto& v = g_var->getValue(); \
        for(auto& i : v) { \
            QLC_LOG_INFO(QLC_LOG_ROOT()) << #prefix " " #name ": " << i; \
        } \
        QLC_LOG_INFO(QLC_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }
    #define XX_M(g_var, name, prefix) \
    { \
        auto& v = g_var->getValue(); \
        for(auto& i : v) { \
            QLC_LOG_INFO(QLC_LOG_ROOT()) << #prefix " " #name ": {" \
                    << i.first << " - " << i.second << "}"; \
        } \
        QLC_LOG_INFO(QLC_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }
    #define XX_P(g_var, name, prefix) \
    { \
        auto& v = g_var->getValue(); \
            QLC_LOG_INFO(QLC_LOG_ROOT()) << v.toString(); \
        QLC_LOG_INFO(QLC_LOG_ROOT()) << #prefix " " #name " yaml: " << g_var->toString(); \
    }
    Person_value_config->addListener([](const Person &old,const Person &new_value){
        std::cout<<"listter里的输入输出"<<std::endl;
        QLC_LOG_INFO(QLC_LOG_ROOT()) << "改变前的数据： "<<old.toString()<< "改变后的数据: "<<new_value.toString() ;
    });
    // XX(vec_value_config, int_vec, before);
    // XX_M(map_value_config, int_map, before);
    XX_P(Person_value_config,int_person ,before);
    YAML::Node root = YAML::LoadFile("../log.yml");
    qlc::Config::LoadFromYaml(root);

    // XX(vec_value_config, int_vec, after);
    // XX_M(map_value_config, int_vec, after);
    XX_P(Person_value_config,int_person ,after);
    // QLC_LOG_INFO(QLC_LOG_ROOT()) << "after: " << int_value_config->getValue();
    // QLC_LOG_INFO(QLC_LOG_ROOT()) << "after: " << float_value_config->toString();   
}

void test_log() {
    static qlc::Logger::ptr system_log = QLC_LOG_NAME("System");
    QLC_LOG_INFO(system_log) << "hello system" << std::endl;
    std::cout << qlc::Loggermgr::GetInstanceX()->toYamlString() << std::endl;
    YAML::Node root = YAML::LoadFile("../log.yml");
    qlc::Config::LoadFromYaml(root);
    std::cout << "=============" << std::endl;
    std::cout << qlc::Loggermgr::GetInstanceX()->toYamlString() << std::endl;
    std::cout << "=============" << std::endl;
    std::cout << root << std::endl;
    QLC_LOG_INFO(system_log) << "hello system" << std::endl;

    system_log->setFormatter("%d - %m%n");
    QLC_LOG_INFO(system_log) << "hello system" << std::endl;
}



int main(int argc,char** agrv){

    QLC_LOG_DEBUG(QLC_LOG_ROOT())<<int_value_config->toString();
    test_log();
    qlc::Config::Visit([](qlc::ConfigVarBase::ptr var) {
        QLC_LOG_INFO(QLC_LOG_ROOT()) << "name=" << var->getName()
                    << " description=" << var->getDescription()
                    << " typename=" << var->getTypeName()
                    << " value=" << var->toString();
    });
}
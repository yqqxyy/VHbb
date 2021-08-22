/* Dear emacs, this is -*-c++-*- */
#ifndef _OvCompat_H_
#define _OvCompat_H_

#include <stdexcept>
#include <sstream>
#include <memory>
#include <iostream>
#include <cassert>

#include "ConfigStore.h"
#include "EventLoop/IWorker.h"

namespace NTA {
  class Message {
  public:
    enum ESeverity {kVERBOSE, kDEBUG3, kDEBUG2, kDEBUG1, kDEBUG, kINFORMATION, kUNKNOWN, kWARNING,  kERROR, kTEST, kFATAL};
  };

  class StreamHelper {
  public:
    template <class T>
    StreamHelper &operator<<(const T &a) {
      m_stream << a;
      return *this;
    }
    std::string str() { return m_stream.str(); }
  private:
    std::stringstream m_stream;
  };

}
#define UNUSED(expr) (void)(expr)

#define NTA_MAKE_MESSAGE_STR(_severity, _name, _id, _message) (NTA::StreamHelper() << #_severity << " " << _name << " " << _message ).str()
#define NTA_MESSAGE_STR(_severity, _level, _name, _id, _message) { if (_severity>=_level ) std::cerr << #_severity << " " << _name << " " << _message << std::endl; }
#define NTA_MSG_DEBUG_STR(_dummy_svc_ptr, _level, _name , _id ,_message)  NTA_MESSAGE_STR(NTA::Message::kDEBUG,_level,_name,_id,_message )
#define NTA_MSG_VERBOSE_STR(_dummy_svc_ptr, _level, _name , _id ,_message)  NTA_MESSAGE_STR(NTA::Message::kVERBOSE,_level,_name,_id,_message )
#define NTA_MSG_INFORMATION_STR(_dummy_svc_ptr, _level, _name , _id ,_message)  NTA_MESSAGE_STR(NTA::Message::kINFORMATION,_level,_name,_id,_message )
#define NTA_MSG_WARNING_STR(_dummy_svc_ptr, _level, _name , _id ,_message)  NTA_MESSAGE_STR(NTA::Message::kWARNING,_level,_name,_id,_message )
#define NTA_MSG_ERROR_STR(_dummy_svc_ptr, _level, _name , _id ,_message)  NTA_MESSAGE_STR(NTA::Message::kERROR,_level,_name,_id,_message )
#define NTA_MSG_FATAL_STR(_dummy_svc_ptr, _level, _name , _id ,_message)  NTA_MESSAGE_STR(NTA::Message::kFATAL,_level,_name,_id,_message )
// dummy implementations to facilitate Overkill to CxAODFramework migration.
namespace Ov
{

  template <class T_Interface, class T_InterfaceBase=T_Interface>
  class ReflectionLayer : public virtual T_Interface
  {
  public:
    typedef T_Interface      InterfaceType_t;
    typedef T_InterfaceBase  InterfaceBaseType_t;
  };

  class IAnalysisMain {
  public:
    virtual ~IAnalysisMain() {}    
    virtual void setConfigStore(ConfigStore *the_config_store) = 0;
    virtual ConfigStore *configStore() = 0;
    virtual void setWorker(EL::IWorker *wk) = 0;
    virtual EL::IWorker *worker() = 0;
  };

  class AnalysisContext : public virtual IAnalysisMain {
  public:
    AnalysisContext( ConfigStore *the_config_store, EL::IWorker *wk=nullptr) : m_configStore(the_config_store), m_worker(wk) {}
    AnalysisContext() : m_configStore(nullptr), m_worker(nullptr) {}

    void setConfigStore(ConfigStore *the_config_store) {
      m_configStore=the_config_store;
    }

    ConfigStore *configStore() {
      return m_configStore;
    }

    void setWorker(EL::IWorker *wk) { m_worker = wk; }

    EL::IWorker *worker() { return m_worker; }

  private:
    
    ConfigStore *m_configStore;
    EL::IWorker      *m_worker;
  };

  
  std::pair<TFile *, bool> getFile( IAnalysisMain &analysis_context, const char *name);

  class ITool {
  public:
    virtual ~ITool() {}
    virtual void initialize(IAnalysisMain *) = 0;    
    virtual void finalize(IAnalysisMain *) = 0;
  };

  class IAnalysisMain;

  class IToolKit {
  public:
    virtual ~IToolKit() {}
    virtual ITool *create(const std::string &type_name, const std::string &name, IAnalysisMain *analysis_main) const = 0;
  };

  class ToolKitManager
  {
  public:
    ToolKitManager() {}
    ~ToolKitManager() {
      for ( std::pair<std::string const,IToolKit *> name_kit :  m_kits) {
        delete name_kit.second;
      }
      m_kits.clear();
    }

    void registerToolKit( const std::string &a_name, IToolKit *a_kit ) { m_kits.insert( std::make_pair(a_name, a_kit) ); }

    template <class T_ToolInterface> 
    std::unique_ptr<T_ToolInterface> create( const std::string &a_name, IAnalysisMain *analysis_main) {
      std::string::size_type pos = a_name.find("/");
      std::string type_name( ( pos == std::string::npos ? a_name : a_name.substr(0,pos)) );
      std::map<std::string,IToolKit *>::const_iterator iter = m_kits.find( type_name );
      if (iter == m_kits.end()) {
        std::stringstream message;
        message << "No tool kit for tool " << type_name << " (" << a_name << ").";
        throw std::runtime_error( message.str() );
      }
      std::unique_ptr<ITool> a_tool( iter->second->create( type_name,
                                           ( pos == std::string::npos ? a_name : a_name.substr(pos+1, a_name.size()-pos-1) ),
                                           analysis_main) );
      T_ToolInterface *the_tool(dynamic_cast<T_ToolInterface *>(a_tool.get()));
      if (!the_tool) {
        std::stringstream message;
        message << "No tool kit " << iter->first << " for tool " << a_name << " has not the correct interface.";
        throw std::runtime_error( message.str() );
      }
      a_tool.release();
      return std::unique_ptr<T_ToolInterface>(the_tool);
    }

    template <class T_ToolInterface> 
    static void createAndSet( const std::string &a_name, IAnalysisMain *analysis_main, std::unique_ptr<T_ToolInterface> &ptr) {
      ptr = std::move( instance()->create<T_ToolInterface>(a_name, analysis_main));
    }

    static ToolKitManager *instance() {
      if (!s_instance) {
        s_instance = new ToolKitManager;
      }
      return s_instance;
    }

  private:
      std::map<std::string,IToolKit *> m_kits;
    static ToolKitManager *s_instance;
  };

  template <class T_dummy, class T_Tool>
  class ToolKit : public virtual IToolKit 
  {
  public:
    ITool *create(const std::string &type_name, const std::string &name, IAnalysisMain *analysis_main) const {
      return new T_Tool(type_name, name, analysis_main);
    }
  };

  class  RuntimeIssue : public std::runtime_error 
  {
  public:
    RuntimeIssue(std::string message)  : std::runtime_error(message) {}
  };

  class Config {
  public:
    Config (const std::string &name, ConfigStore *the_config_store)
      : m_name(name),
        m_configStore(the_config_store),
        m_dumpValues(false)
    { m_configStore->getif<bool>("DumpOvConfig", m_dumpValues); }

    template <class T>
    void registerConfigValue(const std::string &name,
                             const std::string &comment,
                             T &val);

    const std::string &name() const {return m_name; }

    std::string typeName() const {
      std::string::size_type pos = m_name.find("/");
      return ( pos == std::string::npos ? m_name : m_name.substr(0,pos)) ;
    }

  private:
    std::string m_name;    
    ConfigStore *m_configStore;
    bool m_dumpValues;
  };

  class EventInfo_t {
  public:
    EventInfo_t() : m_eventNumber(0), m_runNumber(0) {}
    EventInfo_t(ITool &/*dummy*/) : m_eventNumber(0), m_runNumber(0) {}

    EventInfo_t(unsigned int event_number, unsigned int run_number) : m_eventNumber(event_number), m_runNumber(run_number) {}
    void set (unsigned int event_number, unsigned int run_number) { m_eventNumber=event_number;m_runNumber=run_number;}
    
    unsigned int eventNumber() const { return m_eventNumber; }
    unsigned int runNumber()   const { return m_eventNumber; }

  private:
    unsigned int m_eventNumber;
    unsigned int m_runNumber;
  };


  template <class T>
  class GlobalToolKit : public ToolKit<void, T> {
  public:
  };


  template <class T_Kit >
  class KitRegistrar {
  public:
    KitRegistrar(const std::string &a_name) {
      ToolKitManager::instance()->registerToolKit(a_name, new T_Kit );
    }
  };



  class Tool : public virtual ITool {
  public:
    Tool(const std::string &tool_type_name, const std::string &name, IAnalysisMain *analysis_main) 
      : m_config(tool_type_name + "/" + name, 
                 (analysis_main
                  ? analysis_main->configStore()
                  : nullptr) ) 
    {}

    const std::string name() const { return m_config.name() ; }

    void initialize(IAnalysisMain *) {}   
    void finalize(IAnalysisMain *) {}

  protected:
    Config &config() { return m_config; }
    const Config &config() const { return m_config; }

    template <class T_container_1, class T_container_2>
    bool checkDimensions(const T_container_1 &a, const T_container_2 &b) {
      if (a.size() != b.size()) {
        std::stringstream message;
        message << name() << ": container size mismatch: " << a.size() << " != " << b.size();
        throw std::runtime_error(message.str()  );
      }
      return true;
    }

    
    Config m_config;
  };

  class VerboseTool : public Tool {
  public:
    VerboseTool(const std::string &tool_type_name, const std::string &name, IAnalysisMain *analysis_main) 
  : Tool(tool_type_name,name, analysis_main) {
      m_config.registerConfigValue("MessageLevel","The message output level ",(m_msgLvl=static_cast<int>(NTA::Message::kINFORMATION)));
    }

    void *msgSvc() { return nullptr; }
    int   msgLvl() { return m_msgLvl; }
  private:
    int m_msgLvl;
  };


  template <class T>
  std::ostream &operator<<(std::ostream &out, const std::vector<T> &vec) {
    for (auto elm : vec) {
      out << elm << " ";
    }
    return out;
  }

  template <class T>
  inline void Config::registerConfigValue(const std::string &key_name,
                                   const std::string &,
                                   T &val) {
    if (m_configStore) {
      m_configStore->getif<T>("GLOBAL."+key_name, val);
      std::string type_name = this->typeName();
      if (type_name != this->name() ) {
        m_configStore->getif<T>(type_name+"."+key_name, val);
      }
      m_configStore->getif<T>(this->name()+"."+key_name, val);
      if (this->m_dumpValues) {
        std::cout << "INFO Config " << this->name() << " " << key_name << " = " << val << std::endl;
      }
    }
  }
  
}
#endif

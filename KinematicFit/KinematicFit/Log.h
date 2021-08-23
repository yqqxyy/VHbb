/**
 * @class Log
 * @brief A generic logger class
 * @author Manuel Proissl <mproissl@cern.ch>
 */

#ifndef __LOG_H__
#define __LOG_H__

#include <string>
#include <sstream>
#include <iostream>
#include <stdio.h>

//Log levels
enum TLogLevel {FATAL, ERROR, WARNING, INFO, VERBOSE, DEBUG, DEBUG1, DEBUG2};

//Global class wide settings ~~~~~>
struct LogDef {
  std::string fClassName;
  TLogLevel fPrintLevel;

  LogDef(std::string name, TLogLevel level) {
    fClassName = name;
    fPrintLevel = level;
  }
  std::string GetClassName() {
    return fClassName;
  }
  TLogLevel GetPrintLevel() {
    return fPrintLevel;
  }
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~<

class Log
{
 public:
  Log();
  Log(LogDef* def);
  virtual ~Log();

  std::ostringstream& Write(TLogLevel level = INFO);

 protected:
  std::ostringstream os;

 private:
  LogDef* logdef;
  Log(const Log&);
  Log& operator =(const Log&);

 private:
  bool fWrite;
  static std::string ToString(TLogLevel level);
  static std::string GetClassStr(std::string str);
  TLogLevel msgLevel;
};

//Default constructor
//___________________________________________________________________________________
inline Log::Log()
{
  fWrite=false;
  logdef=NULL;
}//;

//Custom constructor
//___________________________________________________________________________________
inline Log::Log(LogDef* def)
{
  fWrite=false;
  logdef=def;
}//;

//STD output stream
//___________________________________________________________________________________
inline std::ostringstream& Log::Write(TLogLevel level)
{
  fWrite=true;
  msgLevel=level;

  if(logdef->fClassName.empty()) logdef->fClassName="Log::?";

  os << GetClassStr(logdef->fClassName) << "(" << ToString(level) << ") ";
    
  return os;
}//;

//Convert enum to string
//___________________________________________________________________________________
inline std::string Log::ToString(TLogLevel level)
{
  static const char* const buffer[] = {"FATAL", "ERROR", "WARNING", "INFO", "VERBOSE", "DEBUG", "DEBUG1", "DEBUG2"};
  return buffer[level];
}//;

//Make standard class name prefix
//___________________________________________________________________________________
inline std::string Log::GetClassStr(std::string str)
{
  //15 character prefix standard
  if(int(str.size())>15) {
    str=str.substr(0,15);
    str=str+std::string("...   ");
    return str;
  }
  else if(int(str.size())==15) {
    str=str+std::string("      ");
    return str;
  }
  int q=21-int(str.size());
  for(int i=0;i<q;++i)
    str=str+std::string(" ");
  return str;
}//;

//Write log message to screen (or file)
//___________________________________________________________________________________
inline Log::~Log()
{
  if(fWrite && msgLevel <= logdef->fPrintLevel)
    {
      os << std::endl;
      fprintf(stderr, "%s", os.str().c_str());
      fflush(stderr);
    }
}//;

#endif //__LOG_H__

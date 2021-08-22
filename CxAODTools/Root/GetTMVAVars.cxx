#include "CxAODTools/GetTMVAVars.h"
#include <iostream>
#include <TList.h>
#include <TXMLAttr.h>
#include <stdexcept>
#include <sstream>
#include <cstdlib>

void GetTMVAVars::OnEndDocument()
{
  if (m_nVars != m_nExpectedVars) {
    std::stringstream message;
    message << "Expected number of variables does not match the number of variables (" << m_nExpectedVars << "!= " << m_nVars << ".";
    throw std::runtime_error( message.str() );
  }
  for(std::vector<std::string>::const_iterator iter = m_variables.begin();
      iter != m_variables.end();
      ++iter) {
    if (iter->empty()) {
      std::stringstream message;
      message << "Empty variable (index" << (iter - m_variables.begin()) << "<? " << m_variables.size() << ").";
      throw std::runtime_error( message.str() );
    }
  }
}

void GetTMVAVars::OnStartElement(const char* name, const TList* attr) {
  if (!attr || !name) return;

  if (strcmp(name,"Variables")==0) {
    TListIter iter(attr);
    TObject *obj;
    while ((obj=iter.Next())) {
      if (obj && obj->InheritsFrom(TXMLAttr::Class())) {
        TXMLAttr *xml_attr = static_cast<TXMLAttr *>(obj);
        if (strcmp( xml_attr->GetName(),"NVar")==0) {
          m_nExpectedVars=atoi(xml_attr->GetValue());

          if (m_nExpectedVars>1000) {
            std::stringstream message;
            message << "Unreasonably huge number of input variables (NVar=" << m_nExpectedVars << ").";
            throw std::runtime_error(message.str() );
          }

          m_variables.resize(m_nExpectedVars);
        }
      }
    }
  }
  else if (strcmp(name,"Variable")==0) {
    TListIter iter(attr);
    TObject *obj;
    std::string variable_name;
    unsigned int variable_index=m_variables.size();

    while ((obj=iter.Next())) {
      if (obj && obj->InheritsFrom(TXMLAttr::Class())) {
        TXMLAttr *xml_attr = static_cast<TXMLAttr *>(obj);
        if (strcmp(xml_attr->GetName(),"Expression")==0) {
          variable_name = xml_attr->GetValue();
            
        }
        else if (strcmp(xml_attr->GetName(),"Type")==0) {
          if (strcmp(xml_attr->GetValue(),"F")!=0) {
            std::stringstream message;
            message << "Unhandled input variable type (" << xml_attr->GetValue() << "). Expected \"F\".";
            throw std::runtime_error(message.str());
          }
            
        }
        else if (strcmp(xml_attr->GetName(),"VarIndex")==0) {
          variable_index = atoi(xml_attr->GetValue());
        }
        //          std::cout << name << " " << xml_attr->GetName() << "=" << xml_attr->GetValue() << endl;
      }
    }
    if (variable_index>=m_variables.size()) {
      std::stringstream message;
      message << "Variable index out of range (" << variable_index << "< " << m_variables.size() <<")..";
      throw std::runtime_error(message.str() );
    }
    if (variable_name.empty()) {
      std::stringstream message;
      message << "Variable name is empty (index=" << variable_index << ").";
      throw std::runtime_error( message.str() );
    }
    ++m_nVars;
    m_variables[variable_index] = variable_name;
  }
}

GetTMVAVars::operator bool() const {
  if (m_variables.size() != m_nExpectedVars) return false;
  for(std::vector<std::string>::const_iterator iter = m_variables.begin();
      iter != m_variables.end();
      ++iter) {
    if (iter->empty()) return false;
  }
  return true;
}


void GetTMVAVars::dump()
{
  for(std::vector<std::string>::const_iterator iter = m_variables.begin();
      iter != m_variables.end();
      ++iter) {
    std::cout << *iter << " ";
  }
  std::cout << std::endl;
}

//ClassImp(GetTMVAVars)


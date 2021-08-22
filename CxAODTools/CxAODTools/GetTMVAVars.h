#ifndef _GetTMVAVars_h_
#define _GetTMVAVars_h_

#include <TSAXParser.h>
#include <vector>
#include <string>

/** Class to parse a TMVA weight file and extract the list of variables.
 * The class parses the TMVA weight file (tested with TMVA 4.1.4) and extracts the 
 * variables which are used in the classifier in the correct order. 
 */
class GetTMVAVars : public TSAXParser
{
public :
  GetTMVAVars() {reset(); }

  /** Parse TMVA weight file and extract the variables used in the classifier.
   */
  GetTMVAVars(const char *file_name) { reset(); ParseFile(file_name); }

  void	OnStartElement(const char* name, const TList* attr);
  
  void	OnEndDocument();

  /** Reset internal arrays.
   * Should be called before @ref ParseFile is called.
   */
  void reset() {
    m_nVars=0;
    m_nExpectedVars=0;
    m_variables.clear();
  }

  /** Check whether the extracted variable list is consistent.
   * @return true if the vriable list is consistent.
   */
  operator bool() const;

  /** Get the list of variables used in the classifier."
   */
  const std::vector<std::string> &variables() const {
    return m_variables;
  }

  /** Dump the extracted variable list to the screen.
   */
  void dump();

private:
  unsigned int m_nVars;
  unsigned int m_nExpectedVars;

  std::vector<std::string> m_variables;
//  ClassDef(GetTMVAVars,0)
};

#endif


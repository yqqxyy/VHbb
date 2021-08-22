// Dear emacs, this is -*-c++-*-
#ifndef CxAODTools_CutFlowCounter_H
#define CxAODTools_CutFlowCounter_H

#include <map>

// ROOT includes
#include "TString.h"
#include "TH1D.h"

class CutFlowCounter {

  public: 

    // Counter for cut flow
    struct Counter {

      // cut order 
      int priority;

      // number of events 
      double count;

      // sum of event weight
      double sumOfWeight;

    };

    // name of the cut flow
    TString m_name;

    // cut flow information: TString->Cut name ; Counter-> see above
    std::map<std::string, Counter> m_cutFlowInfor;

    // keep track of the priority of the previous cut
    int m_previousPriority;

    // Empty constructor
    CutFlowCounter() {};

    // Constructor: set the name of the cutflow, e.g. electron or event level;
    CutFlowCounter( TString name ) ;

    // Set the name of cut one by one to setup the cutflowInfor map 
    void setCutFlow( TString cutName, int priority ) ;

    // Count the number of entries and weights passing the cut
    // If priority is negative use m_previousPriority + 1
    void count( TString cutName, int priority = -1, double weight=1 ) ;

    // Don't count in some case, e.g. systematics variations and etc
    TH1D * getCutFlow( TString prefix = "" ) ;

};

#endif

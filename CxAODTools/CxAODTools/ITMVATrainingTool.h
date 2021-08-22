/* Dear emacs, this is -*-c++-*- */
#ifndef _ITMVATrainingTool_H_
#define _ITMVATrainingTool_H_

#include "OvCompat.h"
using namespace Ov;
#ifndef _OvCompat_H_
#include <ITool.h>
#endif
#include <TMVA/Types.h>

class ITMVATrainingTool : virtual public ITool
{

public:
   
   virtual void addEvent(std::vector<double>& vars,
                         double weight,
                         TMVA::Types::ESBType eventClass = TMVA::Types::kSignal, TMVA::Types::ETreeType eventType = TMVA::Types::kMaxTreeType) const = 0;

   virtual void addEvent(int eventNumber,
		   	   	   	   	 std::vector<double>& vars,
                         double weight,
                         TMVA::Types::ESBType eventClass = TMVA::Types::kSignal, TMVA::Types::ETreeType eventType = TMVA::Types::kMaxTreeType) const = 0;

   virtual unsigned int getNVars() const = 0;
   virtual unsigned int getNTargets() const = 0;
   virtual unsigned int getNCategoryMarkers() const = 0;
   virtual unsigned int getNCategories() const = 0;
   virtual const std::vector<std::string>& getVariableList() const = 0;
};

#endif

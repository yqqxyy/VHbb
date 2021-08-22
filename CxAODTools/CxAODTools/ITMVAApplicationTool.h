/* Dear emacs, this is -*-c++-*- */
#ifndef _ITMVAApplicationTool_H_
#define _ITMVAApplicationTool_H_

#include "OvCompat.h"
using namespace Ov;

#ifndef _OvCompat_H_
#include <ITool.h>
#endif

class ITMVAApplicationTool : virtual public ITool
{

public:
   
   virtual std::vector<std::vector<float> > evaluateRegression(std::vector<double>& vars) const = 0;
   virtual std::vector<std::vector<float> > evaluateRegression(int eventnumber, std::vector<double>& vars) const = 0;
   virtual std::vector<float> evaluateClassification(std::vector<double>& vars) const = 0;
   virtual float evaluateClassification(std::vector<double>& vars, const std::string& methodName) const = 0;
   virtual std::vector<float> evaluateMulticlass(std::vector<double>& vars, unsigned int cls) const = 0;
   virtual std::vector<std::vector<float> > evaluateMulticlass(std::vector<double>& vars) const = 0;
   virtual std::vector<float> getSignalProbability(std::vector<double>& vars, double expectedSignalFraction = 0.5) const = 0;
   virtual std::vector<float> getBackgroundRarity(std::vector<double>& vars) const = 0;
  
   virtual unsigned int getNVars() const = 0;
   virtual unsigned int getNCategoryMarkers() const = 0;
   virtual const std::vector<std::string>& getVariableList() const = 0;
   virtual const std::vector<std::string>& getMethodList() const = 0;
   virtual unsigned int getNMethods() const = 0;
};

#endif

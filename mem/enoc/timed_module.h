

#ifndef _TIMED_MODULE_H_
#define _TIMED_MODULE_H_

#include "module.h"

class TimedModule : public Module {

public:
  TimedModule(Module * parent, string const & name) : Module(parent, name) {}
  virtual ~TimedModule() {}
  
  virtual void ReadInputs(int global_clk) = 0;
  virtual void clk(int global_clk)  = 0;
  virtual void WriteOutputs(int global_clk) = 0;
};

#endif

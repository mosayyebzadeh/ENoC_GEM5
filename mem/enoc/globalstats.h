

#ifndef __GLOBALSTATS_H__
#define __GLOBALSTATS_H__

#include "statsNoC.h"
#include "Network.h"
//#include "router.h"
 
//#include "NoCMain.h"

class GlobalStats {
  
  private:
      NetworkNoC * _net;
  public:
      GlobalStats( NetworkNoC * _noc);
      unsigned int getReceivedFlits();
      void showStats(std::ostream & out = std::cout);
      unsigned int getReceivedPackets();
      unsigned long long int getNumberFlips();
      unsigned int getNumberFlips_Num(int flipnum); 
      double getStaticEnergy();
      double getDynamicEnergy();
      double getAverageDelay();  
      
      double Get_link_dyn_energy();
      double Get_buff_dyn_energy();
      double Get_xbar_dyn_energy();
      double Get_VCA_dyn_energy();
      double Get_SWA_dyn_energy();       
   double  Get_head_flit_dynamic_energy();   
   double  Get_body_flit_dynamic_energy();  
   double  Get_tail_flit_dynamic_energy();  

   double  Get_Num_head_flit();   
   double  Get_Num_body_flit();  
   double  Get_Num_tail_flit();           
};

#endif

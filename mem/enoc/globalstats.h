

#ifndef __GLOBALSTATS_H__
#define __GLOBALSTATS_H__

#include "statsNoC.h"
#include "Network.h"
//#include "router.h"
 
//#include "NoCMain.h"
#include "config.h"
#include "statsNoC.h"

class GlobalStats {
  
  private:
      NetworkNoC * _net;
      int _total_sims;
  public:
      GlobalStats( const Configuration &config,NetworkNoC * _noc);
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
   
   double  Get_dynamic_energy_for_packet( int pk_type);
   double  Get_link_energy_for_packet( int pk_type);
   double  Get_Flips_energy_for_packet( int pk_type);
};

#endif

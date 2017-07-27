#ifndef __STATS_H__
#define __STATS_H__

#include <vector>
#include "NoCMain.h"
#include "flit.h"
#include "energy.h"
 
//using namespace std;

#define Num_Packet_Types   9

struct CommHistory {
    
    int                 src_id;
    vector < double >   delays;
    unsigned int        total_received_flits;
    unsigned int        total_received_packets;
    unsigned long long int                 flip_numbers;
    
    double              link_dynamic_energy;
    double              buffer_dynamic_energy;    
    double              crossbar_dynamic_energy;        
    double              vc_allocator_dynamic_energy;        
    double              sw_allocator_dynamic_energy; 

     

};


struct packet_info {
    
    unsigned long long int                 flip_numbers;
    
    double              dynamic_energy;
    
    double              link_dynamic_energy;
    double              buffer_dynamic_energy;    
    double              crossbar_dynamic_energy;        
    double              vc_allocator_dynamic_energy;        
    double              sw_allocator_dynamic_energy;     
    
};
    

class StatsNoC {
    
  public:
    
   Energy energy_router;   

    double              head_flit_dynamic_energy;   
    double              body_flit_dynamic_energy;  
    double              tail_flit_dynamic_energy; 

    double              Num_head_flit;   
    double              Num_body_flit;  
    double              Num_tail_flit;     
    
  public:
      
   StatsNoC();
   unsigned int getReceivedFlits();
   void receivedFlit(const int arrival_time, const Flit * flit);
   unsigned int getReceivedPackets();
   unsigned long long int getNumberFlips();
   unsigned int getNumberFlips_Num(int flipnum);
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

    int id;
   vector<CommHistory> chist;
   int searchCommHistory(int src_id);  
   
   vector<packet_info> pkt_info;
};

#endif


#include "router.h"
#include "globalstats.h"
#include <fstream>

GlobalStats::GlobalStats( const Configuration &config,NetworkNoC * _noc)
{
    _net = _noc;
    
     //_total_sims = config.GetInt("_total_sims");
     _total_sims = _total_sims_def;

}


unsigned int GlobalStats::getReceivedFlits(){
    
    unsigned int n = 0;
  
    vector<ENoCRouter *> _ru = _net->GetRouters();
    int    num_routers = _net->NumRouters( );
    
    for ( int i=0; i<num_routers; i++)
		n += _ru[i]->stats.getReceivedFlits();
    
    return n;
}

unsigned int GlobalStats::getReceivedPackets(){
    unsigned int n = 0;

    vector<ENoCRouter *> _ru = _net->GetRouters();
    int    num_routers = _net->NumRouters( );
    
	for ( int i=0; i<num_routers; i++)
		n += _ru[i]->stats.getReceivedPackets();
    return n;
}

unsigned long long int GlobalStats::getNumberFlips()
{
    unsigned long long int n = 0;
        
    vector<ENoCRouter *> _ru = _net->GetRouters();
    int    num_routers = _net->NumRouters( );
    
	for ( int i=0; i<num_routers; i++)    
            	n += _ru[i]->stats.getNumberFlips();
        
         

    return n;
}

unsigned int GlobalStats::getNumberFlips_Num(int flipnum)
{
    unsigned int n = 0;
        
    vector<ENoCRouter *> _ru = _net->GetRouters();
    int    num_routers = _net->NumRouters( );
    
	for ( int i=0; i<num_routers; i++)  
	        	
	      n += _ru[i]->stats.getNumberFlips_Num(flipnum);
 
    return n;

}
double GlobalStats::getAverageDelay()
{
    double avg_delay = 0.0;
    int count = 0;
    double tmp;
        
    vector<ENoCRouter *> _ru = _net->GetRouters();
    int    num_routers = _net->NumRouters( );
    
    for ( int i=0; i<num_routers; i++){

        tmp =  _ru[i]->stats.getAverageDelay();////
        avg_delay += tmp;
        if(tmp != 0)
           count++;
    }

    return avg_delay/(double)count;
}

double GlobalStats::getStaticEnergy(){
    double n = 0;

    vector<ENoCRouter *> _ru = _net->GetRouters();
    int    num_routers = _net->NumRouters( );
    
	for ( int i=0; i<num_routers; i++)
		n += _ru[i]->GetStaticEnergy();
    return n;
}

double GlobalStats::getDynamicEnergy(){
    double n = 0;

    vector<ENoCRouter *> _ru = _net->GetRouters();
    int    num_routers = _net->NumRouters( );
    
	for ( int i=0; i<num_routers; i++)
		n += _ru[i]->GetDynamicEnergy();
    return n;
}

double GlobalStats::Get_link_dyn_energy(){

    double n = 0;

    vector<ENoCRouter *> _ru = _net->GetRouters();
    int    num_routers = _net->NumRouters( );
    
	for ( int i=0; i<num_routers; i++)
		n += _ru[i]->stats.Get_link_dyn_energy();
    return n;

  
}
double GlobalStats::Get_buff_dyn_energy(){

    double n = 0;

    vector<ENoCRouter *> _ru = _net->GetRouters();
    int    num_routers = _net->NumRouters( );
    
	for ( int i=0; i<num_routers; i++)
		n += _ru[i]->stats.Get_buff_dyn_energy();
    return n;      
}
double GlobalStats::Get_xbar_dyn_energy(){

    double n = 0;

    vector<ENoCRouter *> _ru = _net->GetRouters();
    int    num_routers = _net->NumRouters( );
    
	for ( int i=0; i<num_routers; i++)
		n += _ru[i]->stats.Get_xbar_dyn_energy();
    return n;     
}
double GlobalStats::Get_VCA_dyn_energy(){

    double n = 0;

    vector<ENoCRouter *> _ru = _net->GetRouters();
    int    num_routers = _net->NumRouters( );
    
	for ( int i=0; i<num_routers; i++)
		n += _ru[i]->stats.Get_VCA_dyn_energy();
    return n;       
}
double GlobalStats::Get_SWA_dyn_energy(){

    double n = 0;

    vector<ENoCRouter *> _ru = _net->GetRouters();
    int    num_routers = _net->NumRouters( );
    
	for ( int i=0; i<num_routers; i++)
		n += _ru[i]->stats.Get_SWA_dyn_energy();
    return n;       
}

double  GlobalStats::Get_head_flit_dynamic_energy()
{
    double n = 0;

    vector<ENoCRouter *> _ru = _net->GetRouters();
    int    num_routers = _net->NumRouters( );
    
    for ( int i=0; i<num_routers; i++)
        n += _ru[i]->stats.Get_head_flit_dynamic_energy();
    return n;  
}
double  GlobalStats::Get_body_flit_dynamic_energy()
{
    double n = 0;

    vector<ENoCRouter *> _ru = _net->GetRouters();
    int    num_routers = _net->NumRouters( );
    
    for ( int i=0; i<num_routers; i++)
        n += _ru[i]->stats.Get_body_flit_dynamic_energy();
    return n;  
} 
double  GlobalStats::Get_tail_flit_dynamic_energy()
{
    double n = 0;

    vector<ENoCRouter *> _ru = _net->GetRouters();
    int    num_routers = _net->NumRouters( );
    
    for ( int i=0; i<num_routers; i++)
        n += _ru[i]->stats.Get_tail_flit_dynamic_energy();
    return n;  

}  

double  GlobalStats::Get_Num_head_flit()
{
    double n = 0;

    vector<ENoCRouter *> _ru = _net->GetRouters();
    int    num_routers = _net->NumRouters( );
    
    for ( int i=0; i<num_routers; i++)
        n += _ru[i]->stats.Get_Num_head_flit();
    return n;  

}   
double  GlobalStats::Get_Num_body_flit()
{
    double n = 0;

    vector<ENoCRouter *> _ru = _net->GetRouters();
    int    num_routers = _net->NumRouters( );
    
    for ( int i=0; i<num_routers; i++)
        n += _ru[i]->stats.Get_Num_body_flit();
    return n;  

}
double  GlobalStats::Get_Num_tail_flit()
{
    double n = 0;

    vector<ENoCRouter *> _ru = _net->GetRouters();
    int    num_routers = _net->NumRouters( );
    
    for ( int i=0; i<num_routers; i++)
        n += _ru[i]->stats.Get_Num_tail_flit();
    return n;  

}  

double  GlobalStats::Get_dynamic_energy_for_packet( int pk_type)
{
    double n = 0;

    vector<ENoCRouter *> _ru = _net->GetRouters();
    int    num_routers = _net->NumRouters( );
    
    for ( int i=0; i<num_routers; i++)
        n += _ru[i]->stats.pkt_info[pk_type].dynamic_energy;
    return n;  

}
double  GlobalStats::Get_link_energy_for_packet( int pk_type)
{
    double n = 0;

    vector<ENoCRouter *> _ru = _net->GetRouters();
    int    num_routers = _net->NumRouters( );
    
    for ( int i=0; i<num_routers; i++)
        n += _ru[i]->stats.pkt_info[pk_type].link_dynamic_energy;
    return n;  

}

double  GlobalStats::Get_Flips_energy_for_packet( int pk_type)
{
    double n = 0;

    vector<ENoCRouter *> _ru = _net->GetRouters();
    int    num_routers = _net->NumRouters( );
    
    for ( int i=0; i<num_routers; i++)
        n += _ru[i]->stats.pkt_info[pk_type].flip_numbers;
    return n;  

}

void GlobalStats::showStats(std::ostream & out)
{
    
/*	  ofstream fout;
	  fout.open("flip.txt", ios::app);
	  
	  int n = 0;
	  int sum = 0;
	  for (int y = 0; y < 1000000000; y++)
	  {
	        n = getNumberFlips_Num(y);
	        sum = sum + y*n;
	        
	        if ( n != 0)
	            fout << " Total Flips " << std::dec << y  << " : " << n  << endl;
	   
	  }
	  
	  fout << " sum  "  << std::dec << sum <<  endl;
	  fout.close();    
    
*/
		double Total_flits			  =  getReceivedFlits();
		double Total_packets		  =  getReceivedPackets();

        double Total_StaticEnergy     =  getStaticEnergy(); 

        unsigned long long int    NumberFlips            =  getNumberFlips() ;
        
        double DynEnergy              =  getDynamicEnergy();
        double Link_Dyn_Energy        =  NumberFlips * BIT_TRANSITION_DYNAMIC_ENERGY;
        double Total_Dyn_Energy       =  DynEnergy + Link_Dyn_Energy;

        double Total_Energy           =  Total_StaticEnergy + Total_Dyn_Energy; 
        
        
        double All_Flits_Link_Dyn_Energy    = Get_link_dyn_energy();
        double All_Flits_Buff_Dyn_Energy    = Get_buff_dyn_energy();        
        double All_Flits_Xbar_Dyn_Energy    = Get_xbar_dyn_energy();          
        double All_Flits_VCA_Dyn_Energy     = Get_VCA_dyn_energy();   
        double All_Flits_SWA_Dyn_Energy     = Get_SWA_dyn_energy();    
        //double All_Flits_Dyn_Energy         = All_Flits_Link_Dyn_Energy + All_Flits_Buff_Dyn_Energy + All_Flits_Xbar_Dyn_Energy +                   All_Flits_VCA_Dyn_Energy + All_Flits_SWA_Dyn_Energy;   


        double Avg_Flits_Link_Dyn_Energy    = All_Flits_Link_Dyn_Energy / Total_flits ;
        double Avg_Flits_Buff_Dyn_Energy    = All_Flits_Buff_Dyn_Energy / Total_flits;        
        double Avg_Flits_Xbar_Dyn_Energy    = All_Flits_Xbar_Dyn_Energy / Total_flits;          
        double Avg_Flits_VCA_Dyn_Energy     = All_Flits_VCA_Dyn_Energy  / Total_flits;   
        double Avg_Flits_SWA_Dyn_Energy     = All_Flits_SWA_Dyn_Energy  / Total_flits;    
        double Avg_Flits_Dyn_Energy         = Avg_Flits_Link_Dyn_Energy + Avg_Flits_Buff_Dyn_Energy + Avg_Flits_Xbar_Dyn_Energy +                   Avg_Flits_VCA_Dyn_Energy + Avg_Flits_SWA_Dyn_Energy;   


     out << "% Total received flits:                            "  << std::dec << Total_flits << endl;
     out << "% Total received packets:                          "  << std::dec << Total_packets << endl;        
     out << "% Total Flips:                                     "  << std::dec << NumberFlips << endl; 
     out << endl  ; 
     out << "------    Network Energy Report ----------      " << endl  ;
     out << "% Total Static  Energy in Routers (j) :            "  << std::dec << Total_StaticEnergy  << endl;       
     out << "% Link  Dynamic Energy (j) :                       "  << std::dec << Link_Dyn_Energy << endl; 
     out << "% Router Dynamic  Energy (j) :                     "  << std::dec << DynEnergy  << endl;         
     out << "% Total Dynamic Energy (j) :                       "  << std::dec << Total_Dyn_Energy  << endl; 
     out << "% Total Energy (j) :                               "  << std::dec << Total_Energy << endl;      
     out << "% Percentage ( Link Energy / Total Dynamic Energy):"  << std::dec << (Link_Dyn_Energy/Total_Dyn_Energy) * 100 << endl;     
     out << "% Percentage ( Link Energy / Total Energy):        "  << std::dec << (Link_Dyn_Energy/Total_Energy) * 100;
     out << endl << endl ;        
     
     out << "------  Flit Dynamic Energy Report per flit----------      " << endl  ;     
     out << "% Avg Link    Energy (j) :                   "  << std::dec << Avg_Flits_Link_Dyn_Energy  << endl;
     out << "% Avg Buffer  Energy (j) :                   "  << std::dec << Avg_Flits_Buff_Dyn_Energy  << endl;
     out << "% Avg Xbar    Energy (j) :                   "  << std::dec << Avg_Flits_Xbar_Dyn_Energy  << endl;
     out << "% Avg VC Allocator  Energy (j) :             "  << std::dec << Avg_Flits_VCA_Dyn_Energy   << endl;
     out << "% Avg SW Allocator  Energy (j) :             "  << std::dec << Avg_Flits_SWA_Dyn_Energy   << endl;
     out << "% Percentage ( Avg Link Energy / Avg Flit Energy):    "  << std::dec << (Avg_Flits_Link_Dyn_Energy/Avg_Flits_Dyn_Energy) * 100;

    // out << "% Total Dynamic Energy for all flits (j) :     "  << std::dec << All_Flits_Dyn_Energy << endl;    
    // out << "% Percentage ( Link Energy / Total Energy):    "  << std::dec << (All_Flits_Link_Dyn_Energy/All_Flits_Dyn_Energy) * 100;
     
     out << endl << endl;
     out << "------     Timing Report ----------      "  << endl  ;
     out << "% Total Average Delay (second):                    "  << std::dec << getAverageDelay() * CYCLE_TIME  << endl;  
     out << "% Total simulation cycles (second):                "  << std::dec << _total_sims * CYCLE_TIME << endl;


     out << "------  Flit Breakdown Energy Report----------      " << endl  ;     
      out << "% Avg Head Flit    Energy (j) :                   "  << std::dec << Get_head_flit_dynamic_energy()/Get_Num_head_flit()  << endl;
      out << "% Avg body Flit    Energy (j) :                   "  << std::dec << Get_body_flit_dynamic_energy()/Get_Num_body_flit()  << endl;
      out << "% Avg tail Flit    Energy (j) :                   "  << std::dec << Get_tail_flit_dynamic_energy()/Get_Num_tail_flit()  << endl;

			out << "------  Packet Breakdown Energy Report----------      " << endl  ;     
			for (int i =0 ; i < Num_Packet_Types ; i++)
			{
				
				
  	    out << "% Packet type "<< i << " Dynamic Energy (j) :                   "  << std::dec << Get_dynamic_energy_for_packet(i)<< endl;
	      out << "% Packet tyoe "<< i << " link Energy (j) :                      "  << std::dec << Get_link_energy_for_packet(i) << endl;
  	    out << "% Packet type "<< i << " Flips Energy (j) :                     "  << std::dec << Get_Flips_energy_for_packet(i)  << endl;



			}



}



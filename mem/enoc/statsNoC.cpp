

#include "statsNoC.h"
#include "flit.h"


StatsNoC::StatsNoC()
{
	chist.resize(2);
	//chist[0] = new CommHistory();


    head_flit_dynamic_energy = 0;   
    body_flit_dynamic_energy = 0;  
    tail_flit_dynamic_energy = 0; 

    Num_head_flit  = 0;   
    Num_body_flit  = 0;  
    Num_tail_flit  = 0;   
    
    pkt_info.resize(Num_Packet_Types);
    
    for (unsigned int i = 0; i < Num_Packet_Types; i++)
    {
        pkt_info[i].flip_numbers                    =  0;
        pkt_info[i].link_dynamic_energy             =  0;        
        pkt_info[i].buffer_dynamic_energy           =  0;
        pkt_info[i].crossbar_dynamic_energy         =  0;
        pkt_info[i].vc_allocator_dynamic_energy     =  0;
        pkt_info[i].sw_allocator_dynamic_energy     =  0;
      }
}
int StatsNoC::searchCommHistory(int src_id)
{
    for (unsigned int i = 0; i < chist.size(); i++)
	if (chist[i].src_id == src_id)
	    return i;

    return -1;
}


void StatsNoC::receivedFlit(const int arrival_time, const Flit * flit)
{
	int i = searchCommHistory(flit->src_id);


    if (i == -1) {
        // first flit received from a given source
        // initialize CommHist structure
        CommHistory ch;

        ch.src_id = flit->src_id;
        ch.total_received_flits = 0;
        ch.total_received_packets = 0;
    //masoud
        ch.flip_numbers = flit->flipNumber;
        
        ch.link_dynamic_energy          = flit->link_dynamic_energy ; //Get_link_dyn_energy();
        ch.buffer_dynamic_energy        = SWITCHING_ACTIVITY * flit->buffer_dynamic_energy;//Get_buff_dyn_energy();    
        ch.crossbar_dynamic_energy      = SWITCHING_ACTIVITY * flit->crossbar_dynamic_energy;//Get_xbar_dyn_energy();        
        ch.vc_allocator_dynamic_energy  = SWITCHING_ACTIVITY * flit->vc_allocator_dynamic_energy;//Get_VCA_dyn_energy();        
        ch.sw_allocator_dynamic_energy  = SWITCHING_ACTIVITY * flit->sw_allocator_dynamic_energy;//Get_SWA_dyn_energy();
        

      //  cout << " ch.flip_numbers: " << ch.flip_numbers <<  endl;

        chist.push_back(ch);

        i = chist.size() - 1;
    }
    else
    {
    	chist[i].flip_numbers += flit->flipNumber;
//    	cout << " ch.flip_numbers: " << chist[i].flip_numbers <<  endl;
        
        chist[i].link_dynamic_energy          += flit->link_dynamic_energy;//Get_link_dyn_energy();
        chist[i].buffer_dynamic_energy        += SWITCHING_ACTIVITY * flit->buffer_dynamic_energy;//Get_buff_dyn_energy();    
        chist[i].crossbar_dynamic_energy      += SWITCHING_ACTIVITY * flit->crossbar_dynamic_energy;//Get_xbar_dyn_energy();        
        chist[i].vc_allocator_dynamic_energy  += SWITCHING_ACTIVITY * flit->vc_allocator_dynamic_energy;//Get_VCA_dyn_energy();        
        chist[i].sw_allocator_dynamic_energy  += SWITCHING_ACTIVITY * flit->sw_allocator_dynamic_energy;//Get_SWA_dyn_energy();        
    }

     if (flit->flit_type == FLIT_TYPE_HEAD){
	chist[i].total_received_packets++;
       }
       
       chist[i].delays.push_back(arrival_time - flit->timestamp);
       chist[i].total_received_flits++;


       double flit_energy = flit->link_dynamic_energy + SWITCHING_ACTIVITY * (flit->buffer_dynamic_energy + flit->crossbar_dynamic_energy+ flit->vc_allocator_dynamic_energy + flit->sw_allocator_dynamic_energy);

       switch(flit->flit_type)
       {
            case FLIT_TYPE_HEAD :
                head_flit_dynamic_energy += flit_energy ; 
                Num_head_flit++;   

                break; 
            case FLIT_TYPE_BODY : 
                body_flit_dynamic_energy += flit_energy ; 
                Num_body_flit++;  
                
                break; 
            case FLIT_TYPE_TAIL:
                tail_flit_dynamic_energy += flit_energy ;
                Num_tail_flit++;  
                break;                 

       }

        pkt_info[flit->packet_type].flip_numbers                    +=  flit->flipNumber;
        pkt_info[flit->packet_type].link_dynamic_energy             +=  flit->link_dynamic_energy;        
        pkt_info[flit->packet_type].buffer_dynamic_energy           +=  SWITCHING_ACTIVITY * flit->buffer_dynamic_energy;
        pkt_info[flit->packet_type].crossbar_dynamic_energy         +=  SWITCHING_ACTIVITY * flit->crossbar_dynamic_energy;
        pkt_info[flit->packet_type].vc_allocator_dynamic_energy     +=  SWITCHING_ACTIVITY * flit->vc_allocator_dynamic_energy;
        pkt_info[flit->packet_type].sw_allocator_dynamic_energy     +=  SWITCHING_ACTIVITY * flit->sw_allocator_dynamic_energy;
        
        pkt_info[flit->packet_type].dynamic_energy                  +=  flit_energy;
}


unsigned int StatsNoC::getReceivedFlits()
{
    int n = 0;

    for (unsigned int i = 0; i < chist.size(); i++)
        n += chist[i].total_received_flits;

    return n;
}

unsigned int StatsNoC::getReceivedPackets()
{
    int n = 0;

    for (unsigned int i = 0; i < chist.size(); i++)
	n += chist[i].total_received_packets;//chist[i].delays.size();

    return n;
}

unsigned long long int StatsNoC::getNumberFlips()
{
    unsigned long long int n = 0;

    for (unsigned int i = 0; i < chist.size(); i++)
	n += chist[i].flip_numbers;

    return n;
}

unsigned int StatsNoC::getNumberFlips_Num(int flipnum)
{
    int n = 0;

    for (unsigned int i = 0; i < chist.size(); i++)
    {
 	if(flipnum == chist[i].flip_numbers)
        {
            n++;
        }
    }
    return n;
}


double StatsNoC::getAverageDelay()
{
    double sum = 0.0;
    int total_flits = getReceivedFlits();
    //int i = searchCommHistory(src_id);

        if( total_flits == 0)
            return 0;

    for (unsigned int i = 0; i < chist.size(); i++)
        for (unsigned int j = 0; j < chist[i].delays.size(); j++)
                sum += chist[i].delays[j];

   
    return sum / total_flits;
}


double StatsNoC::Get_link_dyn_energy(){

    double n = 0;

    for (unsigned int i = 0; i < chist.size(); i++)
	n += chist[i].link_dynamic_energy; 

    return n;    
}
double StatsNoC::Get_buff_dyn_energy(){

    double n = 0;

    for (unsigned int i = 0; i < chist.size(); i++)
	n += chist[i].buffer_dynamic_energy; 

    return n;        
}
double StatsNoC::Get_xbar_dyn_energy(){

    double n = 0;

    for (unsigned int i = 0; i < chist.size(); i++)
	n += chist[i].crossbar_dynamic_energy; 

    return n;        
}
double StatsNoC::Get_VCA_dyn_energy(){

    double n = 0;

    for (unsigned int i = 0; i < chist.size(); i++)
	n += chist[i].vc_allocator_dynamic_energy; 

    return n;        
}
double StatsNoC::Get_SWA_dyn_energy(){

    double n = 0;

    for (unsigned int i = 0; i < chist.size(); i++)
	n += chist[i].sw_allocator_dynamic_energy; 

    return n;        
}

   double  StatsNoC::Get_head_flit_dynamic_energy()
   {
        return head_flit_dynamic_energy;
   }
   double  StatsNoC::Get_body_flit_dynamic_energy()
   {
        return body_flit_dynamic_energy;
   } 
   double  StatsNoC::Get_tail_flit_dynamic_energy()
   {
        return tail_flit_dynamic_energy;

   }  
   double  StatsNoC::Get_Num_head_flit(){return Num_head_flit;}   
   double  StatsNoC::Get_Num_body_flit(){return Num_body_flit;} 
   double  StatsNoC::Get_Num_tail_flit(){return Num_tail_flit;} 



#ifndef _FLIT_H_
#define _FLIT_H_

#include <iostream>
using namespace std;
#include "NoCMain.h"


typedef unsigned int uint32_t;

class Flit {

    public:

        int           src_id     ;
        int           dst_id     ;
        FlitType      flit_type  ;    // The flit type (FLIT_TYPE_HEAD, FLIT_TYPE_BODY, FLIT_TYPE_TAIL)
        int           sequence_no;    // The sequence number of the flit inside the packet
        uint32_t      payload    ;    // Optional payload
        int        timestamp  ;    // Unix timestamp at packet generation

        int           hop_no     ;    // Current number of hops from source to destination
        bool          watch;
        unsigned long long int           flipNumber ;
        bool          flit_is_inverted;
        int           id;
        int           vc         ;

        int           output_port_vc_start;
        int           output_port_vc_end;
 
        int           output_port;
        int           out_vc;
        
        double           link_dynamic_energy;
        double           buffer_dynamic_energy;    
        double           crossbar_dynamic_energy;        
        double           vc_allocator_dynamic_energy;        
        double           sw_allocator_dynamic_energy;        
        
        int    packet_type;


    public:

      Flit();
    //  ~Flit() {}
    static Flit * New();
    
    void Update_link_dyn_energy(double link_dyn_energy);
    void Update_buff_dyn_energy(double buff_dyn_energy);
    void Update_xbar_dyn_energy(double xbar_dyn_energy);
    void Update_VCA_dyn_energy(double VCA_dyn_energy);
    void Update_SWA_dyn_energy(double SWA_dyn_energy);

    double Get_link_dyn_energy();
    double Get_buff_dyn_energy();
    double Get_xbar_dyn_energy();
    double Get_VCA_dyn_energy();
    double Get_SWA_dyn_energy();

    inline void SetOutputVC( int vcnumber ) 
    {
        out_vc = vcnumber;
    }
    inline int GetOutputVC( ) const
    {
        return out_vc;
    }
  
};

ostream& operator<<( ostream& os, const Flit& f );



#endif

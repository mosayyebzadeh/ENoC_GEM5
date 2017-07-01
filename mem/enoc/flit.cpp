
#include "flit.h"

   Flit::Flit()
{
    src_id      = -1;
    dst_id      = -1;
    timestamp   = -1;
    vc          =  0;

    link_dynamic_energy             = 0;
    buffer_dynamic_energy           = 0;    
    crossbar_dynamic_energy         = 0;        
    vc_allocator_dynamic_energy     = 0;        
    sw_allocator_dynamic_energy     = 0;             
}

ostream& operator<<( ostream& os, const Flit& f )
{

  os << "  Source: " << f.src_id << "  Dest: " << f.dst_id <<	endl;
  os << " Type: " 	 << f.flit_type  << endl;
  os << "  VC: "     << f.vc << endl;
  return os;
}

Flit * Flit::New() {
  Flit * f;
  f = new Flit;
  return f;
}

void Flit::Update_link_dyn_energy(double link_dyn_energy){
    
    link_dynamic_energy += link_dyn_energy;
}
void Flit::Update_buff_dyn_energy(double buff_dyn_energy){
    
    buffer_dynamic_energy += buff_dyn_energy;
}
void Flit::Update_xbar_dyn_energy(double xbar_dyn_energy){
    
    crossbar_dynamic_energy += xbar_dyn_energy;
}
void Flit::Update_VCA_dyn_energy(double VCA_dyn_energy){
    
    vc_allocator_dynamic_energy += VCA_dyn_energy;
}
void Flit::Update_SWA_dyn_energy(double SWA_dyn_energy){
    
    sw_allocator_dynamic_energy += SWA_dyn_energy;
}
double Flit::Get_link_dyn_energy(){
    
    return link_dynamic_energy;
}
double Flit::Get_buff_dyn_energy(){
  
    return buffer_dynamic_energy;
}
double Flit::Get_xbar_dyn_energy(){
    
    return crossbar_dynamic_energy;
}
double Flit::Get_VCA_dyn_energy(){
    
    return vc_allocator_dynamic_energy;
}
double Flit::Get_SWA_dyn_energy(){
    
    return sw_allocator_dynamic_energy;
}

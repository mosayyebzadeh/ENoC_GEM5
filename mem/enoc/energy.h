#ifndef __NOXIMENERGY_H__
#define __NOXIMENERGY_H__

 
#include "NoCMain.h"
#include "element.h"
using namespace std;

class Energy {

  public:
    
    double cycle_time;
    
    double SwitchingActivity;
    
    double dynamic_energy;
    double static_energy;
     
    double AllEnergy;
    
    SWAllocator _swa;
    VCAllocator _vca;
    InputBuffer _inb;
    CrossBar    _xbar;
      
  public:    

    Energy();
 
    void InputBuffer_dynamic_energy_per_vc_access();
    void InputBuffer_static_energy();  
    
    void VCAlocator_dynamic_energy();
    void VCAlocator_static_energy();   

    void SWAlocator_dynamic_energy();
    void SWAlocator_static_energy();  
    
    void CrossBar_dynamic_energy();
    void CrossBar_static_energy();      
    
    double GetStaticEnergy();
    double GetDynamicEnergy();
    
    

};

#endif

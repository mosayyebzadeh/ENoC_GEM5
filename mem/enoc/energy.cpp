

#include "energy.h"


Energy::Energy()
{

    cycle_time  = CYCLE_TIME;
    
    SwitchingActivity = SWITCHING_ACTIVITY;
    
    static_energy  = 0;
    dynamic_energy = 0;
    
}
void Energy::Initialize(const Configuration &config)
{
    _swa.Initialize(config);
    _vca.Initialize(config);
    _xbar.Initialize(config);
}
double Energy::GetStaticEnergy()
{
    return static_energy;
}

double Energy::GetDynamicEnergy()
{
    
    return dynamic_energy;
}

void Energy::InputBuffer_dynamic_energy_per_vc_access()
{
    dynamic_energy  += SwitchingActivity * _inb.getDynamicEnergy();
    
}

void Energy::InputBuffer_static_energy()
{
    static_energy  += _inb.getLeakagePower() * cycle_time;
}


void Energy::VCAlocator_dynamic_energy()
{
    dynamic_energy  += SwitchingActivity *  _vca.getDynamicEnergy();
}


void Energy::VCAlocator_static_energy()
{
    static_energy  += _vca.getLeakagePower()* cycle_time;
}


void Energy::SWAlocator_dynamic_energy()
{
    dynamic_energy  += SwitchingActivity *  _swa.getDynamicEnergy();
}

void Energy::SWAlocator_static_energy()
{
    static_energy  += _swa.getLeakagePower()* cycle_time;
    
}
    
void Energy::CrossBar_dynamic_energy()
{
    dynamic_energy  += SwitchingActivity *  _xbar.getDynamicEnergy();
}
void Energy::CrossBar_static_energy()
{
    static_energy  += _xbar.getLeakagePower()* cycle_time;
}

    
    
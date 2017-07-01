

#ifndef _ELEMENT_HPP
#define _ELEMENT_HPP

class Element
{
protected:
    // unit
    double area;
    double delay;
    double leakage_power;
    double dynamic_energy;


public:
    double getArea() { return  area; }
    double getLeakagePower() { return  leakage_power; }
    double getDynamicEnergy() { return  dynamic_energy; }
    double getDelay() { return delay; }

};

class Inv : public Element
{
public:
    Inv()
    {    
        //area = ;
        delay = SCALING_FACTOR_45n_TO_32nm_LATENCY * 0.01e-9 ;
        leakage_power = SCALING_FACTOR_45n_TO_32nm_POWER * 14.35*1e-9  ;
        dynamic_energy = SCALING_FACTOR_45n_TO_32nm_ENERGY * 1.85*1e-15 ;

    }
};

class AND2_X1 : public Element
{
public:
    AND2_X1()
    {    
        //area = ;
        delay = SCALING_FACTOR_45n_TO_32nm_LATENCY * 0.02*1e-9; //0.02 ns
        leakage_power = SCALING_FACTOR_45n_TO_32nm_POWER * 25.07*1e-9; //25.07 nw
        dynamic_energy = SCALING_FACTOR_45n_TO_32nm_ENERGY * 3.04*1e-15; //3.04 fj

    }
};

class OR2_X1 : public Element
{
public:
    OR2_X1()
    {    
        //area = ;
        delay = SCALING_FACTOR_45n_TO_32nm_LATENCY * 0.04*1e-9;
        leakage_power = SCALING_FACTOR_45n_TO_32nm_POWER * 22.69*1e-9 ;
        dynamic_energy = SCALING_FACTOR_45n_TO_32nm_ENERGY * 3.10*1e-15;

    }
};

class OR4_X1 : public Element
{
public:
    OR4_X1()
    {    
        //area = ;
        delay = SCALING_FACTOR_45n_TO_32nm_LATENCY * 0.11*1e-9;
        leakage_power = SCALING_FACTOR_45n_TO_32nm_POWER * 26.73*1e-9 ;
        dynamic_energy = SCALING_FACTOR_45n_TO_32nm_ENERGY * 4.67*1e-15;

    }
};

class FF_X1 : public Element
{
public:
    FF_X1()
    {    
        //area = ;
        delay = SCALING_FACTOR_45n_TO_32nm_LATENCY * 0.08*1e-9;
        leakage_power = SCALING_FACTOR_45n_TO_32nm_POWER * 79.11*1e-9 ;
        dynamic_energy = SCALING_FACTOR_45n_TO_32nm_ENERGY * 4.36*1e-15;

    }
};

class InputBuffer : public Element
{
    double dynamic_energy_per_vc;
public:
    InputBuffer()
    {    
        //area = ;
        delay = 0.780*1e-9;
        leakage_power = 1.697*1e-7; 
        dynamic_energy_per_vc = 0.745*1e-12;
        
    }
    double getDynamicEnergyPerVC() { return  dynamic_energy_per_vc; }
};

class VCAllocator : public Element
{
   
    
    AND2_X1 and2;
    OR2_X1  or2;
    OR4_X1  or4;    
    Inv  not1;
    FF_X1   ff1;
    
public:
    VCAllocator()
    {    
        //area = ;
        delay = or4.getDelay() + not1.getDelay() + and2.getDelay() + or2.getDelay();
        int leakage_tmp =3*and2.getLeakagePower()+2*or2.getLeakagePower()+2*not1.getLeakagePower()+ff1.getLeakagePower();
        leakage_power = 5*8*leakage_tmp + 8*5*leakage_tmp + 5*or4.getLeakagePower() + 8 * or4.getLeakagePower(); 
        
        int dynamic_tmp = 3*and2.getDynamicEnergy()+2*or2.getDynamicEnergy()+2*not1.getDynamicEnergy()+ff1.getDynamicEnergy();
        dynamic_energy = 5*8*dynamic_tmp + 8*5*dynamic_tmp + 5*or4.getDynamicEnergy() + 8 * or4.getDynamicEnergy();
        
    }
    
};

class SWAllocator : public Element
{

    
    AND2_X1 and2;
    OR2_X1  or2;
    OR4_X1  or4;
    Inv     not1;
    FF_X1   ff1;
    
public:
    SWAllocator()
    {    
        //area = ;
        delay = or4.getDelay() + not1.getDelay() + and2.getDelay() + or2.getDelay();
        int leakage_tmp =3*and2.getLeakagePower()+2*or2.getLeakagePower()+2*not1.getLeakagePower()+ff1.getLeakagePower();
        leakage_power = 5*5*leakage_tmp + 5*or4.getLeakagePower(); 
        
        int dynamic_tmp = 3*and2.getDynamicEnergy()+2*or2.getDynamicEnergy()+2*not1.getDynamicEnergy()+ff1.getDynamicEnergy();
        dynamic_energy = 5*5*dynamic_tmp + 5*or4.getDynamicEnergy();
        
    }
    
};

class CrossBar : public Element
{
public:
    CrossBar()
    {    
        //area = ;
        delay = SCALING_FACTOR_45n_TO_32nm_LATENCY * 0.15*1e-9;
        leakage_power = SCALING_FACTOR_45n_TO_32nm_POWER * 170*143.72*1e-9 ;
        dynamic_energy = SCALING_FACTOR_45n_TO_32nm_ENERGY * 170*15.2*1e-15;

    }
};

#endif

#include "config.h"

#include <stdio.h>
#include <string.h>

#include <cstring>
#include <iostream>
#include <ctype.h>


#include <stdlib.h>

#define MAX_LINE_LEN 100


Configuration::Configuration( )
{ 
	AddStrField( "routing_function", "xy" );
	AddStrField( "signaling", "level" );
	AddStrField( "encoding", "binary" );
	
	
	_int_map["use_noc_latency"] = 1;
	_int_map["dimx"] = 8; //number of routers in X
  _int_map["dimy"] = 8; //number of routers in Y
  _int_map["in_ports"]     = 5;
  _int_map["out_ports"]    = 5;
  _int_map["num_vcs"]      = 8;  
  _int_map["vcs_size"]     = 8;  //per vc buffer size
  _int_map["_total_sims"]  = 1;   // number of simulations to perform


//energy
  
  _float_map["SCALING_FACTOR_45n_TO_32nm_LATENCY"]  = 0.91;   //   
  _float_map["SCALING_FACTOR_45n_TO_32nm_POWER"]  = 0.66;   //
  _float_map["SCALING_FACTOR_45n_TO_32nm_ENERGY"]  = 0.66;   //

  //Inv
  _float_map["InvX1_delay"]  = 0.01e-9;   //     
  _float_map["InvX1_leakagePower"]  = 14.35*1e-9;   //  
  _float_map["InvX1_dynamicEnergy"]  = 1.85*1e-15;   //  

  //AND2_X1
  _float_map["AND2_X1_delay"]  = 0.02e-9;   //     
  _float_map["AND2_X1_leakagePower"]  = 25.07*1e-9;   //  
  _float_map["AND2_X1_dynamicEnergy"]  = 3.04*1e-15;   //       
  
  //OR2_X1
  _float_map["OR2_X1_delay"]  = 0.04e-9;   //     
  _float_map["OR2_X1_leakagePower"]  = 22.69*1e-9;   //  
  _float_map["OR2_X1_dynamicEnergy"]  = 3.10*1e-15;   //   
    
  //OR4_X1
  _float_map["OR4_X1_delay"]  = 0.11e-9;   //     
  _float_map["OR4_X1_leakagePower"]  = 26.73*1e-9;   //  
  _float_map["OR4_X1_dynamicEnergy"]  = 4.67*1e-15;   //   

  //FF_X1
  _float_map["FF_X1_delay"]  = 0.08e-9;   //     
  _float_map["FF_X1_leakagePower"]  = 79.11*1e-9;   //  
  _float_map["FF_X1_dynamicEnergy"]  = 4.36*1e-15;   //    
    
}

void Configuration::AddStrField(string const & field, string const & value)
{
  _str_map[field] = value;
}
string Configuration::GetStr(string const & field) const
{
  map<string, string>::const_iterator match;

  match = _str_map.find(field);
  if(match != _str_map.end()) {
    return match->second;
  } else {
    cout << "Unknown integer field:GetStr " + field << std::endl;
    return "exit";
  }
}

int Configuration::GetInt(string const & field) const
{
  map<string, int>::const_iterator match;

  match = _int_map.find(field);
  if(match != _int_map.end()) {
    return match->second;
  } else {
    cout << "Unknown integer field:GetInt " + field << std::endl;
    return -1;
    //exit(-1);
  }
}
double Configuration::GetFloat(string const & field) const
{  
  map<string,double>::const_iterator match;

  match = _float_map.find(field);
  if(match != _float_map.end()) {
    return match->second;
  } else {
    cout << "Unknown string field:GetFloat  " + field << std::endl;
    return -1;
    //exit(-1);
  }
}

void Configuration::Assign(string const & field, string const & value)
{
  map<string, string>::const_iterator match;
  
  _str_map[field] = value;
  /*
  match = _str_map.find(field);
  if(match != _str_map.end()) {
    _str_map[field] = value;
  } else {
    cout << "Unknown string field:Assign " + field << std::endl;
  }*/
}

void Configuration::Assign(string const & field, int value)
{
  map<string, int>::const_iterator match;
  
  _int_map[field] = value;
  /*
  match = _int_map.find(field);
  if(match != _int_map.end()) {
    _int_map[field] = value;
  } else {
    cout << "Unknown integer field:Assign " + field << std::endl;
  }*/
}

void Configuration::Assign(string const & field, double value)
{
  map<string, double>::const_iterator match;
  _float_map[field] = value;
  /*
  match = _float_map.find(field);
  if(match != _float_map.end()) {
    _float_map[field] = value;
  } else {
    cout << "Unknown float field:Assign " + field << std::endl;
  }*/
}

void Configuration::ParseFile(string const & filename)
{
  if((_config_file = fopen(filename.c_str(), "r")) == 0) {
    std::cout << "Could not open configuration file " << filename << std::endl;
    //exit(-1);
  }

  char * pch;
  char line[ MAX_LINE_LEN ];

   while (fgets( line, MAX_LINE_LEN, _config_file ))
    {
    /* Do something with the line. For example: */
      //cout <<  ++n <<  line << endl;
      
        pch = strtok (line," =");
        if (pch != NULL)
        {
       //   cout << pch << endl;
            char *pch2 = pch;
            pch = strtok (NULL, " =");
         
          
          //cout << "pch2 " << pch2  << endl;
          //cout << "pch " << pch  << endl;
          
          
          if (isdigit(pch[0])){
             // int year = atoi (pch   );
              float f2 = strtof (pch, NULL);
              if(f2 >= 1.0)
              {
                //int digit = atoi (pch   );
                //cout << digit << endl;
                  //cout << (int)f2 << endl;
                  Assign(pch2,(int)f2);
              }
              else
                  Assign(pch2,f2); //cout << f2 << endl ;
              
                //Assign(pch2,year);
        //      cout << year << endl;
          }else
          { Assign(pch2,pch);}
        }        
    }
  

  fclose(_config_file);
  _config_file = 0;
}

/*bool ParseArgs(Configuration * cf, int argc, char * * argv)
{
  bool rc = false;

      // parse config file
      cf->ParseFile( argv[i] );
      ifstream in(argv[i]);
      cout << "BEGIN Configuration File: " << argv[i] << endl;
      while (!in.eof()) {
			char c;
			in.get(c);
			cout << c ;
      }
      cout << "END Configuration File: " << argv[i] << endl;
      rc = true;
    
  

  return rc;
}*/

void Configuration::ShowConfigParameters() {
    
    cout << "  Begin    Configuration Parameters      " << endl;
  
  for(map<string,string>::const_iterator i = _str_map.begin(); i!=_str_map.end(); i++){
     
        if(i->second[0]!='\0'){
          cout <<   i->first    <<  " = "   <<  i->second   <<  ";" <<  endl;
        }
  }
  
   cout << "      End    _str_map   " << endl;
  
  for(map<string, int>::const_iterator i = _int_map.begin(); i!=_int_map.end();  i++){
     cout <<    i->first    <<  " = "   <<  i->second   <<  ";" <<  endl;

  }

     cout << "      End    _int_map   " << endl;
     
  for(map<string, double>::const_iterator i = _float_map.begin(); i!=_float_map.end(); i++){
     cout <<    i->first    <<  " = "   <<  i->second   <<  ";" <<  endl;

  }

    cout << "  End     Configuration Parameters      " << endl; 
}


#include "router.h"

#include <string>
#include <sstream>
#include <iostream>
#include "routefunc.h"

Router::Router( Module *parent, const string & name, int id,int inputs, int outputs ) :
TimedModule( parent, name ), _id( id ), _inputs( inputs ), _outputs( outputs )
{
  //_vcs         = GlobalParams::num_vcs;
  _vcs         = DEFAULT_NUM_VCS;

  // Alloc VC's
  _buf.resize(_inputs);
  for ( int i = 0; i < _inputs; ++i ) {
    ostringstream module_name;
    module_name << "buf_" << i;
    _buf[i] = new Buffer( this, module_name.str( ) );
    module_name.str("");    
  }

    _output_buffer.resize(_outputs); 
    
    _output_channels_last_flit.resize(_outputs); 

//inputs  = 4; 
//outputs = 4;

  // Alloc allocators
  
  int iters = 1 ;

  _vc_allocator = new iSLIP_Sparse( this, "vc_allocator",_vcs*_inputs, _vcs*_outputs,iters );
  _sw_allocator = new iSLIP_Sparse( this, "sw_allocator",_inputs, _outputs,iters );

 // m_CrossbarTable = new Crossbar;

}


Router* Router::NewRouter(Module *parent, const string & name, int id,int inputs, int outputs )
{
       Router *r = NULL;
       r = new Router( parent,  name,  id,  inputs,  outputs);
   //    r->_id = id;

       return r;
}
void Router::AddInputChannel( FlitChannel *channel  )
{
  _input_channels.push_back( channel );
   
  channel->SetSink( this, _input_channels.size() - 1 ) ;
}

void Router::AddOutputChannel( FlitChannel *channel  )
{
  _output_channels.push_back( channel );
  
  channel->SetSource( this, _output_channels.size() - 1 ) ;
}

void Router::ReadInputs(int global_clk)
{
    
  bool have_flits = _ReceiveFlits( );

  _active = _active || have_flits ;
}


bool Router::_ReceiveFlits( )
{
  bool activity = false;
  for(int input = 0; input < _inputs; ++input) { 
    Flit * const f = _input_channels[input]->Receive();
    if(f) {

      _in_queue_flits.insert(make_pair(input, f));
      activity = true;
    }
    
    stats.energy_router.InputBuffer_static_energy();
    stats.energy_router.VCAlocator_static_energy();
    stats.energy_router.SWAlocator_static_energy();
    stats.energy_router.CrossBar_static_energy();
  }
  return activity;
}


void Router::clk(int global_clk)
{
  _InputQueuing(global_clk);
  _vc_allocator->Clear();
  
  _VCAllocEvaluate( );
  stats.energy_router.VCAlocator_dynamic_energy();
  
  _sw_allocator->Clear();
  _SWAllocEvaluate( );
  stats.energy_router.SWAlocator_dynamic_energy();
}



void Router::_InputQueuing(int global_clk)
{
  for(map<int, Flit *>::const_iterator iter = _in_queue_flits.begin(); iter != _in_queue_flits.end();  ++iter) 
  {

    int const input = iter->first;
    assert((input >= 0) && (input < _inputs));

    Flit *  f = iter->second;
    assert(f);

    int const vc = f->vc;
    
    assert((vc >= 0) && (vc < _vcs));

    Buffer * const cur_buf = _buf[input];

    //cur_buf->AddFlit(vc, f);
    
    cur_buf -> Route( vc, _id,  f, input ); 
     
     if(f->output_port == 4)
     {
          
	  _output_buffer[f->output_port].push(f); //output  
          
          
          stats.receivedFlit(global_clk, f);
     }
     else { 
         cur_buf->AddFlit(vc, f);
         stats.energy_router.InputBuffer_dynamic_energy_per_vc_access();
         f->Update_buff_dyn_energy(_inb.getDynamicEnergyPerVC());
         //cur_buf->SetState(vc, VirtualChannel::vc_alloc);
         if((f->flit_type == FLIT_TYPE_HEAD ) && (cur_buf->GetState( vc ) == VirtualChannel::routing))
         {
             if(cur_buf->GetOccupancy( vc ) > 1){
                
		// AMIN_ST  
                std::cout << std::dec << global_clk <<"   Head Flit :  " << "src : " << std::dec << f->src_id;
                std::cout << "  dst : " << f->dst_id << "  " << "id router" << _id << endl;
                 
            }
         
         }
         
        if(f->flit_type == FLIT_TYPE_HEAD ){
            _vc_alloc_vcs.push_back(make_pair(-1, make_pair(make_pair(input, vc), -1)));
            cur_buf->SetState(vc, VirtualChannel::vc_alloc);
            //assert(cur_buf->GetOccupancy( vc ) == 1);
        }
        _sw_alloc_vcs.push_back(make_pair(-1, make_pair(make_pair(input, vc), -1)));
     }
 
  }
  _in_queue_flits.clear();

}

int Router::FindIdleVC(int out_port , int dst_id)
{
        
        Router const *r = _output_channels[out_port]->GetSink();
        VirtualChannel::VCState state ;
        int in_port;
        
        if(out_port == 0)
            in_port = 1;
        else if(out_port == 1)
            in_port = 0;
        else if(out_port == 2)
            in_port = 3 ;
        else
            in_port = 2;
        
        for ( int vc_number = 0; vc_number < _vcs; ++vc_number ){
             state = r->_buf[in_port]->GetState( vc_number );

                if (state == VirtualChannel::idle){
                    
                       if(r->GetID() != dst_id)
                            r->_buf[in_port]->SetState( vc_number, VirtualChannel::routing );
                        
                        return vc_number;
                }
                

        }
        return -1;
              
			
}

void Router::SetIdleVC(int out_port, int vc_number )
{

	Router const *r = _output_channels[out_port]->GetSink();
	//VirtualChannel::VCState state;
	int in_port;

	if (out_port == 0)
		in_port = 1;
	else if (out_port == 1)
		in_port = 0;
	else if (out_port == 2)
		in_port = 3;
	else
		in_port = 2;

	r->_buf[in_port]->SetState(vc_number, VirtualChannel::idle);


}

void Router::_VCAllocEvaluate( )
{
    deque<pair<int, pair<pair<int, int>, int> > >::iterator iter;
	
    int size_vc_alloc  =    _vc_alloc_vcs.size();
    
    int  out_vc;
    
    for( iter = _vc_alloc_vcs.begin(); iter != _vc_alloc_vcs.end(); ++iter) 
    {

        int const input = iter->second.first.first;
        int const vc = iter->second.first.second;

        Buffer const * const cur_buf = _buf[input];

        Flit  *  f = cur_buf->FrontFlit(vc);
        
        int const out_port = f->output_port;
        


	/*int out_vc = -1;
        Router const *r = _output_channels[out_port]->GetSink();
   
                int in_port;
                if(out_port == 0)
                    in_port = 1;
                else if(out_port == 1)
                    in_port = 0;
                else if(out_port == 2)
                    in_port = 3 ;
                else
                    in_port = 2;
                
                int occup_tmp = r->_buf[in_port]->GetOccupancy(0);
		int occup;
                for ( int vc_number = 1; vc_number < _vcs; ++vc_number ){
                    occup = r->_buf[in_port]->GetOccupancy( vc_number );
			if (occup <= occup_tmp)
			{
				out_vc = vc_number;
				occup_tmp = occup;
			}
                }

                if(out_vc != -1)
                {
                    if(r->_buf[in_port]->GetOccupancy( out_vc ) >= 5)
                    {
                        out_vc = -1;
                    }
                    
                }                
        */
       
       out_vc = FindIdleVC(out_port, f->dst_id);
       iter->first =   out_vc;
       
        if(out_vc != -1)
        {
		//f->out_vc = out_vc;
                //f->SetOutputVC( out_vc );
            
            
		int input_and_vc = input*_vcs + vc;
                int in_priority = 0;
                int out_priority = 0;
                _vc_allocator->AddRequest(input_and_vc, out_port*_vcs + out_vc, 0, in_priority, out_priority);
                f->Update_VCA_dyn_energy(_vca.getDynamicEnergy()/size_vc_alloc);
        }
    }    
    
    _vc_allocator->Allocate();
    
    while(!_vc_alloc_vcs.empty()) {

        pair<int, pair<pair<int, int>, int> > const & item = _vc_alloc_vcs.front();
        _vc_alloc_vcs.pop_front();

        int const input = item.second.first.first;
        int const vc = item.second.first.second;

        Buffer  *  cur_buf = _buf[input];

        Flit const * const f = cur_buf->FrontFlit(vc);
        
        int const out_port = f->output_port;
        
        int input_and_vc = input*_vcs + vc;
        
        int const output_and_vc = _vc_allocator->OutputAssigned(input_and_vc);
        if( cur_buf->GetState(vc) == (VirtualChannel::vc_alloc))
        {
            if(output_and_vc >= 0) {

                int const match_output = output_and_vc / _vcs;
                int const match_vc = output_and_vc % _vcs;
                cur_buf->SetState(vc, VirtualChannel::active);
                cur_buf->SetOutput(vc, match_output, match_vc);
//		_vc_allocator->RemoveRequest(input_and_vc, f->output_port*_vcs + f->out_vc, 0);
                
                
                    assert(match_output == f->output_port);
                   
                    
                //_vc_allocator->RemoveRequest(input_and_vc, match_output*_vcs + match_vc, 0);
                //_sw_alloc_vcs.push_back(make_pair(-1, make_pair(item.second.first, -1)));
                
            }else{
                
                if(item.first != -1 )
                    SetIdleVC( out_port, item.first);// f->GetOutputVC( ) );
                
                _vc_alloc_vcs_tmp.push_back(make_pair(-1, make_pair(item.second.first, -1)));
                cur_buf->SetState(vc, VirtualChannel::vc_alloc);
            }
        }

    }       
		deque<pair<int, pair<pair<int, int>, int> > >::iterator it = _vc_alloc_vcs.begin();
		_vc_alloc_vcs.insert(it, _vc_alloc_vcs_tmp.begin(), _vc_alloc_vcs_tmp.end());
		_vc_alloc_vcs_tmp.clear();     
}
void Router::_SWAllocEvaluate( )
{
    
  deque<pair<int, pair<pair<int, int>, int> > >::iterator iter;  
  
  int size_sw_alloc  =    _sw_alloc_vcs.size();
   
  for(iter = _sw_alloc_vcs.begin(); iter != _sw_alloc_vcs.end(); ++iter) 
  {

    int const input = iter->second.first.first;
    int const vc = iter->second.first.second;

    Buffer  *  cur_buf = _buf[input];
    
    Flit   *   f = cur_buf->FrontFlit(vc);      
    if(cur_buf->GetState(vc) == VirtualChannel::active)
    {          
      int const dest_output = cur_buf->GetOutputPort(vc);
     // int const dest_vc = cur_buf->GetOutputVC(vc);
      
      int input_and_vc = input;
      int in_priority = 0;
      int out_priority = 0;
      
      _sw_allocator->AddRequest(input_and_vc, dest_output, 0, in_priority, out_priority);
      f->Update_SWA_dyn_energy(_swa.getDynamicEnergy());

    }
  }
      _sw_allocator->Allocate();
  
    while(!_sw_alloc_vcs.empty()) {

        pair<int, pair<pair<int, int>, int> > const & item = _sw_alloc_vcs.front();
        _sw_alloc_vcs.pop_front();

        int const input = item.second.first.first;
        int const vc = item.second.first.second;

        Buffer  *  cur_buf = _buf[input];

        Flit *  f = cur_buf->FrontFlit(vc);
              
         if(cur_buf->GetState(vc) == VirtualChannel::active)
         {
             
            int const out_port = f->output_port;
            int input_and_vc = input;
            int const dest_vc = cur_buf->GetOutputVC(vc);
            
            int const output_and_vc = _sw_allocator->OutputAssigned(input_and_vc);
            
            bool freespace = FindFreeSpace( out_port, dest_vc);
        
            if((output_and_vc >= 0) && (freespace == true)) {
                
                 
                f->vc = cur_buf->GetOutputVC(f->vc) ;// f->out_vc;
                cur_buf->RemoveFlit(vc);
                stats.energy_router.InputBuffer_dynamic_energy_per_vc_access();
                f->Update_buff_dyn_energy(_inb.getDynamicEnergyPerVC()/size_sw_alloc);
                
                if(f->flit_type == FLIT_TYPE_TAIL ){
                        cur_buf->SetState(vc, VirtualChannel::idle);
                        _vc_allocator->RemoveRequest(input*_vcs + vc, out_port*_vcs + dest_vc, 0);
                        //assert(cur_buf->GetOccupancy( vc ) == 0);
                }
                
               _output_buffer[out_port].push(f); //output

                
            }else{
                
                _sw_alloc_vcs_tmp.push_back(make_pair(-1, make_pair(item.second.first, -1)));
                cur_buf->SetState(vc, VirtualChannel::active);
            }
         }
 
    }   
		deque<pair<int, pair<pair<int, int>, int> > >::iterator it = _sw_alloc_vcs.begin();
		_sw_alloc_vcs.insert(it, _sw_alloc_vcs_tmp.begin(), _sw_alloc_vcs_tmp.end());
		_sw_alloc_vcs_tmp.clear();    
}

bool Router::FindFreeSpace(int out_port, int vc_number)
{      
        Router const *r = _output_channels[out_port]->GetSink();
        
        int in_port;
        
        if(out_port == 0)
            in_port = 1;
        else if(out_port == 1)
            in_port = 0;
        else if(out_port == 2)
            in_port = 3 ;
        else
            in_port = 2;
        
    
        int size = r->_buf[in_port]->GetOccupancy( vc_number );
        //if(size < GlobalParams::vcs_size)
        if(size < DEFAULT_VCS_SIZE)
                return true;
        else
                return false;	
}

void Router::_SendFlits( )
{
  for ( int output = 0; output < _outputs; ++output ) {
    if ( !_output_buffer[output].empty( ) ) {
        
      Flit * const f = _output_buffer[output].front( );
      Flit * const last_flit = _output_channels_last_flit[output];


      //int signaling  = GlobalParams::signaling;//0;
      //int encoding   = GlobalParams::encoding;//0;
      int signaling  = SIGNALING_LEVEL;//0;
      int encoding   = ENCODING_BINARY;//0;

      Calc_Flips( signaling, encoding, f, last_flit);


      
      /*
      bool  flit_bits[34];
      bool  last_flit_bits[34];
      
      Get_Bits_of_Flits( f,&flit_bits[0] );
      Get_Bits_of_Flits( last_flit, &last_flit_bits[0] );
      
      int n_flip = 0;
      
      for (int index = 0; index < 34 ; index++) {
         if(flit_bits[index] != last_flit_bits[index])
         n_flip++;
      }
      
      f->flipNumber += n_flip;
      */
      
     // Bus_Invert_coding(f, flit_bits, last_flit_bits );
      
      _output_channels_last_flit[output] = f;

      /* if((_output_buffer[output].size() != 1) && (output != 4))
       {
           cout << " size  : " << _output_buffer[output].size() ;
           cout << " output  : "  << output << endl;
       }
      */
      _output_buffer[output].pop( );
      _output_channels[output]->Send( f );
      
      stats.energy_router.CrossBar_dynamic_energy();
      f->Update_xbar_dyn_energy(_xbar.getDynamicEnergy());
      
      //f->Update_link_dyn_energy( n_flip * BIT_TRANSITION_DYNAMIC_ENERGY );
    }
  }
}

void  Router::Calc_Flips(int signaling,int encoding, Flit* cur, Flit* last)
{

      bool  flit_bits[34];
      bool  last_flit_bits[34];
      
      Get_Bits_of_Flits( cur,&flit_bits[0] );
      Get_Bits_of_Flits( last, &last_flit_bits[0] );
      
      int   n_flip = 0;

      int   not_n_flip = 0;
      bool  Not_curr_flit[34];      

      switch(signaling) {

        case 0: // level signaling
              switch ( encoding )
                      {
                      case 0:  // Binary Encoding

                              for (int index = 0; index < 34 ; index++) {
                                 if(flit_bits[index] != last_flit_bits[index])
                                    n_flip++;
                              }  
                              cur->flipNumber += n_flip;  
                              cur->Update_link_dyn_energy( n_flip * BIT_TRANSITION_DYNAMIC_ENERGY );                  

                        break;
                      case 1:  // Bus Invert coding

                              for (int index = 0; index < 34 ; index++) 
                                  Not_curr_flit[index] = ~flit_bits[index];


                              for (int index = 0; index < 34 ; index++) {

                                  if(flit_bits[index] != last_flit_bits[index])
                                              n_flip++;

                                  if(Not_curr_flit[index] != last_flit_bits[index])
                                              not_n_flip++;                                
                              }

                                            
                              if(not_n_flip < n_flip ){                                   
                                      cur->flipNumber  += not_n_flip;
                                      cur->Update_link_dyn_energy( not_n_flip * BIT_TRANSITION_DYNAMIC_ENERGY ); 
                                    }
                              else{                                          
                                      cur->flipNumber  += n_flip;
                                      cur->Update_link_dyn_energy( n_flip * BIT_TRANSITION_DYNAMIC_ENERGY ); 
                                    }

                        break;
                      }        

                break;
        case 1: // transition signaling
              switch ( encoding )
                      {
                      case 0:

                              for (int index = 0; index < 34 ; index++) {
                                 if(flit_bits[index] == true)
                                      n_flip++;
                              }  
                              cur->flipNumber += n_flip;
                              cur->Update_link_dyn_energy( n_flip * BIT_TRANSITION_DYNAMIC_ENERGY );   


                               /*     cout << "  flit_bits     :";
                                    for (int i = 0; i < 34; ++i)
                                      std::cout << flit_bits[i];
                                    cout <<  std::dec << "      n_flip :" << n_flip << endl;                                                   
                                */
                        break;
                      case 1:

                              for (int index = 0; index < 34; index++)
                              {
                                if (flit_bits[index] == true)
                                {
                                  Not_curr_flit[index] = false;
                                  n_flip++;

                                }
                                else
                                {
                                  Not_curr_flit[index] = true;
                                  not_n_flip++;
                                }
                              }

                              if(not_n_flip < n_flip ){                                   
                                      cur->flipNumber  += not_n_flip;
                                      cur->Update_link_dyn_energy( not_n_flip * BIT_TRANSITION_DYNAMIC_ENERGY ); 
                                    }
                              else{                                          
                                      cur->flipNumber  += n_flip;
                                      cur->Update_link_dyn_energy( n_flip * BIT_TRANSITION_DYNAMIC_ENERGY ); 
                                    }
                        break;
                      }  

                break;
      }      

}

void Router::WriteOutputs(int global_clk)
{
  _SendFlits( );
 // _SendCredits( );
}

void Router::From8bitINT(uint8_t c, bool b[8])
{
    for (int i=0; i < 8; ++i){
        b[i] = (c & (1<<i)) != 0;
      //  cout << b[i];
    }
}

void Router::From32bitINT(uint32_t c, bool b[32])
{
    for (int i=0; i < 32; ++i){
        b[i] = (c & (1<<i)) != 0;
      //  cout << b[i];
    }
}

void  Router::Get_Bits_of_Flits(Flit* flit, bool  flit_bits[34])
{

    //bool  flit_bits[34];
    if(flit == NULL)
    {
          for (int i=0; i < 34; ++i)
               flit_bits[i] = false;     
    }
    else
    {
        if(flit->flit_type == FLIT_TYPE_HEAD)  //head
        {
                From8bitINT(flit->src_id,&flit_bits[0]);
                From8bitINT(flit->dst_id,&flit_bits[8]);    

                for (int i=16; i < 32; ++i)
                        flit_bits[i] = false;   
                
                flit_bits[32] = true; 
                flit_bits[33] = false; 
        }
        else if( flit->flit_type == FLIT_TYPE_BODY) //body
        {
                From32bitINT(flit->payload,&flit_bits[0]);
                flit_bits[32] = false; 
                flit_bits[33] = false;   

        }
        else//tail
        {
            for (int i=0; i < 32; ++i)
                    flit_bits[i] = false;   
                
                flit_bits[32] = false; 
                flit_bits[33] = true; 

        }
    }
   // return flit_bits;

}

void  Router::Bus_Invert_coding(Flit* flit, bool  cur_flit[34], bool last_flit[34] )
{
 	int n_flip = 0; 
	int not_n_flip = 0;
						
        bool  Not_curr_flit[34];

				
        for (int index = 0; index < 34 ; index++) 
            Not_curr_flit[index] = ~cur_flit[index];


        for (int index = 0; index < 34 ; index++) {

            if(cur_flit[index] != last_flit[index])
                        n_flip++;

            if(Not_curr_flit[index] != last_flit[index])
                        not_n_flip++;                                
        }

	// cout << "	n_flip :" << n_flip;
	// cout << "	not n_flip :" << not_n_flip  << endl;

                        
        if(not_n_flip < n_flip )
        {
                                                    

                flit->flipNumber  += not_n_flip;

 

            }
            else
            {                                          
                flit->flipNumber  += n_flip;

  						

            }

       // cout << " flips: " << n_flip <<  endl;   
}

double Router::GetStaticEnergy()
{
    return stats.energy_router.GetStaticEnergy();
}
double Router::GetDynamicEnergy()
{
    return stats.energy_router.GetDynamicEnergy();    
}

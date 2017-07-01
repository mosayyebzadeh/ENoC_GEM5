

#include <sstream>

#include "Buffer.h"
#include "NoCMain.h"

Buffer::Buffer( Module *parent, const string& name ) :
Module( parent, name )
 {

  //_vc.resize(GlobalParams::num_vcs);
  _vc.resize(DEFAULT_NUM_VCS);
  //_occupancy.resize(GlobalParams::num_vcs);
  _occupancy.resize(DEFAULT_NUM_VCS);

  //_size = GlobalParams::vcs_size;
  _size = DEFAULT_VCS_SIZE;

  //for(int i = 0; i < GlobalParams::num_vcs; ++i) {
  for(int i = 0; i < DEFAULT_NUM_VCS; ++i) {
      ostringstream vc_name;
      vc_name << "vc_" << i;
      _vc[i] = new VirtualChannel( this, vc_name.str( ) );
  }

}

/*Buffer::~Buffer()
{
  for(vector<VirtualChannel*>::iterator i = _vc.begin(); i != _vc.end(); ++i) {
    delete *i;
  }
}*/

void Buffer::AddFlit( int vc, Flit *f )
{
  if(_occupancy[vc] >= _size) {
    cout << "Flit buffer overflow. " << vc << "  occup :  " <<_occupancy[vc] << "  state :   " << GetState(  vc ) << endl;
  }
  _occupancy[vc]++;
  _vc[vc]->AddFlit(f);
  f->vc = vc;


}



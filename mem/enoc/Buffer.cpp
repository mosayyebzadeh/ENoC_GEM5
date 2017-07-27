

#include <sstream>

#include "Buffer.h"
#include "NoCMain.h"

Buffer::Buffer( const Configuration &config,Module *parent, const string& name ) :
Module( parent, name )
 {

  //int num_vcs = config.GetInt("num_vcs") ;   
  int num_vcs = DEFAULT_NUM_VCS;
  _vc.resize(num_vcs);
  _occupancy.resize(num_vcs);

  //_size = config.GetInt("vcs_size");
  _size = DEFAULT_VCS_SIZE;

  for(int i = 0; i < num_vcs; ++i) {
  //for(int i = 0; i < DEFAULT_NUM_VCS; ++i) {
      ostringstream vc_name;
      vc_name << "vc_" << i;
      _vc[i] = new VirtualChannel( config, this, vc_name.str( ) );
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



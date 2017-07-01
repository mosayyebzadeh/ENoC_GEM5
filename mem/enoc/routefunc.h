

//#ifndef _ROUTEFUNC_H_
//#define _ROUTEFUNC_H_

#include "flit.h"
#include "router.h"
#include <map>
#include <cstdlib>
#include <cassert>

#include "NoCMain.h"

//typedef void (*tRoutingFunction)( const Router *,  Flit *, int in_channel  );


class Router;
//#endif

typedef void (*tRoutingFunction)( const Router *,  Flit *, int in_channel  );
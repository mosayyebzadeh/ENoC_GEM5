


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
    ParseError("Unknown string field: " + field);
    exit(-1);
  }
}

int Configuration::GetInt(string const & field) const
{
  map<string, int>::const_iterator match;

  match = _int_map.find(field);
  if(match != _int_map.end()) {
    return match->second;
  } else {
    ParseError("Unknown integer field: " + field);
    exit(-1);
  }
}
double Configuration::GetFloat(string const & field) const
{  
  map<string,double>::const_iterator match;

  match = _float_map.find(field);
  if(match != _float_map.end()) {
    return match->second;
  } else {
    ParseError("Unknown double field: " + field);
    exit(-1);
  }
}

void Configuration::Assign(string const & field, string const & value)
{
  map<string, string>::const_iterator match;
  
  match = _str_map.find(field);
  if(match != _str_map.end()) {
    _str_map[field] = value;
  } else {
    ParseError("Unknown string field: " + field);
  }
}

void Configuration::Assign(string const & field, int value)
{
  map<string, int>::const_iterator match;
  
  match = _int_map.find(field);
  if(match != _int_map.end()) {
    _int_map[field] = value;
  } else {
    ParseError("Unknown integer field: " + field);
  }
}

void Configuration::Assign(string const & field, double value)
{
  map<string, double>::const_iterator match;
  
  match = _float_map.find(field);
  if(match != _float_map.end()) {
    _float_map[field] = value;
  } else {
    ParseError("Unknown double field: " + field);
  }
}

void Configuration::ParseFile(string const & filename)
{
  if((_config_file = fopen(filename.c_str(), "r")) == 0) {
    cerr << "Could not open configuration file " << filename << endl;
    exit(-1);
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
          cout << pch << endl;
          Assign(pch,)
          pch = strtok (NULL, " =");
        }      
    }
  

  fclose(_config_file);
  _config_file = 0;
}

bool ParseArgs(Configuration * cf, int argc, char * * argv)
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
}
#ifndef _CONFIG_HPP_
#define _CONFIG_HPP_


#include    <cstdio>
#include    <string>
#include    <map>
#include    <vector>
using namespace std;

class Configuration {
  static Configuration * theConfig;
  FILE * _config_file;
  string _config_string;

protected:
  map<string,string> _str_map;
  map<string,int>    _int_map;
  map<string,double> _float_map;
  
public:
  Configuration();

  void AddStrField(string const & field, string const & value);

  void Assign(string const & field, string const & value);
  void Assign(string const & field, int value);
  void Assign(string const & field, double value);

  string GetStr(string const & field) const;
  int GetInt(string const & field) const;
  double GetFloat(string const & field) const;

  void ParseFile(string const & filename);

  
  void ShowConfigParameters();


  inline const map<string, string> & GetStrMap() const {
    return _str_map;
  }
  inline const map<string, int> & GetIntMap() const {
    return _int_map;
  }
  inline const map<string, double> & GetFloatMap() const {
    return _float_map;
  }

  static Configuration * GetTheConfig();

};

bool ParseArgs(Configuration * cf, int argc, char **argv);

vector<string> tokenize_str(string const & data);
vector<int> tokenize_int(string const & data);
vector<double> tokenize_float(string const & data);

#endif

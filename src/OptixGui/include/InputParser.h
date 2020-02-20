#pragma once  /* -*- c++ -*- */

#include <string>
#include <vector>
#include <algorithm>

/* from https://stackoverflow.com/a/868894 */

class InputParser{
public:
  InputParser (int &argc, char **argv){
    for (int i=1; i < argc; ++i)
      this->tokens.push_back(std::string(argv[i]));
  }
  const std::string& getCmdOption(const std::string &option) const{
    std::vector<std::string>::const_iterator itr;
    itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
    if (itr != this->tokens.end() && ++itr != this->tokens.end()){
      return *itr;
    }
    static const std::string empty_string("");
    return empty_string;
  }
  const std::string& getCmdEquivalentsOption(const std::vector<std::string> &optionList) const{
    for (const std::string option : optionList) {
       const std::string &value = getCmdOption(option);
       if (!value.empty()) {
         return value;
       }
    }
    static const std::string empty_string("");
    return empty_string;
  }
  bool cmdOptionExists(const std::string &option) const{
    return std::find(this->tokens.begin(), this->tokens.end(), option)
      != this->tokens.end();
  }
  bool cmdEquivalentsExist(const std::vector<std::string> &optionList) const{
    for (const std::string option : optionList) {
      if (cmdOptionExists(option))
        return true;
    }
    return false;
  }

private:
  std::vector <std::string> tokens;
};

/****
#include <stdexcept>
int main(int argc, char **argv){
  InputParser input(argc, argv);
  bool Qverbose;

  if(input.cmdOptionExists("-h")){
    // Do stuff
  }
  const std::string &filename = input.getCmdOption("-f");
  if (!filename.empty()){
    // Do interesting things ...
  }
  std::vector <std::string> sameOptionList;
  sameOptionList.clear();
  sameOptionList.push_back("-v"); sameOptionList.push_back("--verbose");
  Qverbose = input.cmdEquivalentsExist(sameOptionList);

  sameOptionList.clear();
  sameOptionList.push_back("-w");  sameOptionList.push_back("--width");
  const std::string &winWidth =  input.getCmdEquivalentsOption(sameOptionList);
  try {
    if (!winWidth.empty()){
    }
  } catch (std::invalid_argument const &ex) {
  }
  return 0;
}
*/

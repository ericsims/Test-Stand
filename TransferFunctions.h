#include "fparser4.5.2/fparser.hh"
#include <string>

class TransferFunctions {
  public:
    void addFunction(std::string);
    double callFunction(double);
  private:
    FunctionParser fp;
};
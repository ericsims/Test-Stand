#include  "TransferFunctions.h"

void TransferFunctions::addFunction(std::string funcText) {
      fp.Parse(funcText, "x");
}

double TransferFunctions::callFunction(double data) {
  double variables[1] = { data };
  return fp.Eval(variables);
}
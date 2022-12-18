#include "importmanager.h"
#include <string>

int main(){
  
  const std::string element = "Ca";
  const std::string element_with_charge = "Ca2+";
  const std::string element_with_suffix = "Ca-ion";

  {
    const std::string result = ImportMngr::stripCharge(element_with_charge);
    if (result != element){return -1;}
  }

  {
    const std::string result = ImportMngr::stripCharge(element_with_suffix);
    if (result != element){return -1;}
  }

  return 0;
}

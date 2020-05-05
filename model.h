// class for dealing with program logic
#include <iostream>
#include <vector>

struct Atom;
class Model{
  public:
    void readAtomsFromFile(std::string& filepath);
  private:
    std::vector<Atom> atoms;
};

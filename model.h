// class for dealing with program logic
#include <iostream>
#include <vector>

struct Atom;
class Space;
class Model{
  public:
    void readAtomsFromFile(std::string& filepath);
    // calls the Space constructor and creates a cell containing all atoms. Cell size is defined by atom positions
    void defineCell();
    void debug();
  private:
    std::vector<Atom> atoms;
    Space* cell;
};

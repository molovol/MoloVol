#include <vector>

class Voxel{
  public:
    Voxel access(const short& x, const short& y, const short& z){
      return data[4 * z + 2 * y + x];
    }
    Voxel access(const short& i){
      return data[i];
    }
    char getType(){
      return type;
    }
  private:
    std::vector<Voxel> data; // empty or exactly 8 elements
    char type = 'm';
};

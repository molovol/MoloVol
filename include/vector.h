#ifndef VECTOR_H

#define VECTOR_H

class Vector{
  public:
    Vector(double, double, double);
    Vector(std::array<double,3>);

    Vector add(const Vector&, const Vector&);
    Vector operator+(const Vector&);
    Vector operator-(const Vector&);
    
    Vector scale(const double&);
    Vector operator*(const double&);
  private:
    double x, y, z;
};

#endif

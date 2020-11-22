#ifndef VECTOR_H

#define VECTOR_H

#include <array>

struct Vector{
  public:
    Vector();
    Vector(double, double, double);
    Vector(std::array<double,3>);
    
    void print(); 
    const double& getCoordinate(char) const;
    void setCoordinate(char,const double&);
    
    double length() const;
    Vector normalise() const; 
    Vector normalize() const {return this->normalise();} 
    double angle(const Vector&) const;

    bool isLongerThan(const Vector&) const;
    bool isLongerThan(const double&) const;
    bool isShorterThan(const Vector&) const;
    bool isShorterThan(const double&) const;
    bool isSameLength(const Vector&) const;
    bool isSameLength(const double&) const;
    bool operator>(const Vector&);
    bool operator>(const double&);
    bool operator>=(const Vector&);
    bool operator>=(const double&);
    bool operator<(const Vector&);
    bool operator<(const double&);
    bool operator<=(const Vector&);
    bool operator<=(const double&);
    bool operator==(const Vector&);
    bool operator==(const double&);
    bool operator!=(const Vector&);
    bool operator!=(const double&);

  private:
    double squared() const;
    std::array<double,3> coord;
};

Vector add(Vector, const Vector&);
Vector operator+(const Vector&, const Vector&);
Vector operator-(const Vector&, const Vector&);

Vector scale(Vector, const double&);
Vector scale(const double&, const Vector&);
Vector operator*(const Vector&, const double&);
Vector operator*(const double&, const Vector&);
Vector operator/(const Vector&, const double&);

double dotproduct(const Vector&, const Vector&);
double operator*(const Vector&, const Vector&);

#endif

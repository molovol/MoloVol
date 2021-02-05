#ifndef VECTOR_H

#define VECTOR_H

#include <array>

struct Vector{
  public:
    // constructors
    Vector();
    Vector(double, double, double);
    Vector(std::array<double,3>);
    
    // access
    double& getCoordinate(char);
    double& operator[](char);
    const double& getCoordinate(char) const;
    const double& operator[](char) const;
    void setCoordinate(char,const double&);
    
    // display
    void print() const; 
    
    // isolated operations
    double length() const;
    Vector normalise() const; 
    Vector normalize() const {return normalise();}
    
    // relation to other vector
    double angle(const Vector&) const;
    
    // relation to multiple vectors
    bool isInsideTriangle(const std::array<Vector,3>&) const;

    // comparisons
    bool isLongerThan(const Vector&) const;
    bool isLongerThan(const double&) const;
    bool isShorterThan(const Vector&) const;
    bool isShorterThan(const double&) const;
    bool isSameLength(const Vector&) const;
    bool isSameLength(const double&) const;
    bool operator>(const Vector&) const;
    bool operator>(const double&) const;
    bool operator>=(const Vector&) const;
    bool operator>=(const double&) const;
    bool operator<(const Vector&) const;
    bool operator<(const double&) const;
    bool operator<=(const Vector&) const;
    bool operator<=(const double&) const;
    bool operator==(const Vector&) const;
    bool operator==(const double&) const;
    bool operator!=(const Vector&) const;
    bool operator!=(const double&) const;

  private:
    // data
    std::array<double,3> coord;

    // methods
    double squared() const;
};

// arithmetic operators
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

Vector crossproduct(const Vector&, const Vector&);

double distance(const Vector&, const Vector&);
double distance(const Vector&, const Vector&, const char);

#endif

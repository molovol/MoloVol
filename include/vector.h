#ifndef VECTOR_H

#define VECTOR_H

#include <array>

// Wrapper class for a numeric array of size three.
// Allows easy mathematical operations. Represents a positional
// vector in R3.
struct Vector{
  public:
    // constructors
    Vector();
    Vector(double, double, double);
    Vector(std::array<double,3>);
    Vector(int, int, int);
    
    // access
    double& getCoordinate(char);
    double& operator[](char);
    double getCoordinate(char) const;
    double operator[](char) const;
    void setCoordinate(char,double);
    
    // display
    void print() const; 
    
    // isolated operations
    double length() const;
    Vector normalise() const; 
    Vector normalize() const {return normalise();}
    
    // comparisons
    bool isLongerThan(const Vector&) const;
    bool isLongerThan(double) const;
    bool isShorterThan(const Vector&) const;
    bool isShorterThan(double) const;
    bool isSameLength(const Vector&) const;
    bool isSameLength(double) const;
    bool operator>(const Vector&) const;
    bool operator>(double) const;
    bool operator>=(const Vector&) const;
    bool operator>=(double) const;
    bool operator<(const Vector&) const;
    bool operator<(double) const;
    bool operator<=(const Vector&) const;
    bool operator<=(double) const;
    bool operator==(const Vector&) const;
    bool operator==(double) const;
    bool operator!=(const Vector&) const;
    bool operator!=(double) const;

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

Vector scale(Vector, double);
Vector scale(double, const Vector&);
Vector operator*(const Vector&, double);
Vector operator*(double, const Vector&);
Vector operator/(const Vector&, double);

double dotproduct(const Vector&, const Vector&);
double operator*(const Vector&, const Vector&);

double distance(const Vector&, const Vector&);
double distance(const Vector&, const Vector&, const char);

#endif

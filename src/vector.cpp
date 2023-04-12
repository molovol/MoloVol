#include "vector.h"
#include <iostream>
#include <cmath>

//////////////////
// CONSTRUCTORS //
//////////////////

Vector::Vector(){
  coord = {0,0,0};
}
Vector::Vector(double inp_x, double inp_y, double inp_z){
  coord = {inp_x, inp_y, inp_z};
}
Vector::Vector(std::array<double,3> inp_arr)
  :coord(inp_arr){}

Vector::Vector(int inp_x, int inp_y, int inp_z){
  coord = {static_cast<double>(inp_x), static_cast<double>(inp_y), static_cast<double>(inp_z)};
}

/////////////
// METHODS //
/////////////

void Vector::print() const {
  printf("(%.4f,%.4f,%.4f)\n", coord[0], coord[1], coord[2]);
}

double& Vector::getCoordinate(char i) {
  return (*this)[i];
}

double& Vector::operator[](char i) {
  return coord[i];
}

double Vector::getCoordinate(char i) const {
  return (*this)[i];
}

double Vector::operator[](char i) const {
  return coord[i];
}

void Vector::setCoordinate(char i, double val){
  coord[i] = val;
}

double Vector::squared() const {
  return ((*this) * (*this));
}

double Vector::length() const {
  return pow(this->squared(),0.5);
}

Vector Vector::normalise() const {
  return (*this)/(this->length());
}

double Vector::angle(const Vector& vec) const {
  return acos(this->normalise() * vec.normalise());
}

bool Vector::isLongerThan(const Vector& vecToCompare) const {
  return (this->squared() > vecToCompare.squared());
}

bool Vector::isLongerThan(double valToCompare) const {
  return (this->squared() > (valToCompare*valToCompare));
}

bool Vector::isShorterThan(const Vector& vecToCompare) const{
  return (this->squared() < vecToCompare.squared());
}

bool Vector::isShorterThan(double valToCompare) const{
  return (this->squared() < (valToCompare*valToCompare));
}

bool Vector::isSameLength(const Vector& vecToCompare) const{
  return (this->squared() == vecToCompare.squared());
}

bool Vector::isSameLength(double valToCompare) const{
  return (this->squared() == (valToCompare*valToCompare));
}

bool Vector::operator>(const Vector& vec) const {return this->isLongerThan(vec);}
bool Vector::operator>(double val) const {return this->isLongerThan(val);}
bool Vector::operator>=(const Vector& vec) const {return !this->isShorterThan(vec);}
bool Vector::operator>=(double val) const {return !this->isShorterThan(val);}
bool Vector::operator<(const Vector& vec) const {return this->isShorterThan(vec);}
bool Vector::operator<(double val) const {return this->isShorterThan(val);}
bool Vector::operator<=(const Vector& vec) const {return !this->isLongerThan(vec);}
bool Vector::operator<=(double val) const {return !this->isLongerThan(val);}
bool Vector::operator==(const Vector& vec) const {return this->isSameLength(vec);}
bool Vector::operator==(double val) const {return this->isSameLength(val);}
bool Vector::operator!=(const Vector& vec) const {return !this->isSameLength(vec);}
bool Vector::operator!=(double val) const {return !this->isSameLength(val);}

bool Vector::isInsideTriangle(const std::array<Vector,3>& vec_vertices) const {
  Vector vec_oop = crossproduct(vec_vertices[1]-vec_vertices[0],vec_vertices[2]-vec_vertices[0]);

  bool sign;
  for (int i = 0; i < 3; i++){
    int j = (i+1)%3;
    if(!i){
      sign = std::signbit(crossproduct(vec_oop,vec_vertices[j]-vec_vertices[i])*((*this)-vec_vertices[i]));
    }
    else if(sign != std::signbit(crossproduct(vec_oop,vec_vertices[j]-vec_vertices[i])*((*this)-vec_vertices[i]))){
      return false;
    }
  }
  return true;
}

////////////////
// OPERATIONS //
////////////////

Vector add(Vector lhs, const Vector& rhs){
  for (char i = 0; i<3; i++){
    lhs.setCoordinate(i, lhs.getCoordinate(i)+rhs.getCoordinate(i));
  }
  return lhs;
}

Vector operator+(const Vector& lhs, const Vector& rhs){
  return add(lhs, rhs);
}

Vector operator-(const Vector& lhs, const Vector& rhs){
  return add(lhs, scale(rhs,-1));
}

Vector scale(Vector vec, double scalar){
  for (char i = 0; i<3; i++){
    vec.setCoordinate(i, vec.getCoordinate(i) * scalar);
  }
  return vec;
}

Vector scale(double scalar, const Vector& vec){return scale(vec,scalar);}

Vector operator*(const Vector& vec, double scalar){return scale(vec, scalar);}
Vector operator*(double scalar, const Vector& vec){return scale(vec, scalar);}
Vector operator/(const Vector& vec, double div){return vec*(1/div);}

double dotproduct(const Vector& rhs, const Vector& lhs){
  double retval = 0;
  for (char i = 0; i<3; i++){
    retval += rhs.getCoordinate(i) * lhs.getCoordinate(i);
  }
  return retval;
}

double operator*(const Vector& rhs, const Vector& lhs){return dotproduct(lhs, rhs);}

Vector crossproduct(const Vector& lhs, const Vector& rhs){
  Vector retvec = Vector();
  retvec[0] = lhs[1]*rhs[2] - lhs[2]*rhs[1];
  retvec[1] = lhs[2]*rhs[0] - lhs[0]*rhs[2];
  retvec[2] = lhs[0]*rhs[1] - lhs[1]*rhs[0];
  return retvec;
}

double distance(const Vector& start, const Vector& end){
  return (end-start).length();
}

double distance(const Vector& start, const Vector& end, const char dim){
  return end[dim] - start[dim];
}

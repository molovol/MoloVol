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

/////////////
// METHODS //
/////////////

void Vector::print(){
  printf("(%.4f,%.4f,%.4f)\n", coord[0], coord[1], coord[2]);
}

const double& Vector::getCoordinate(char i) const {
  return coord[i];
}

void Vector::setCoordinate(char i, const double& val){
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

bool Vector::isLongerThan(const double& valToCompare) const {
  return (this->squared() > (valToCompare*valToCompare)); 
}

bool Vector::isShorterThan(const Vector& vecToCompare) const{
  return (this->squared() < vecToCompare.squared()); 
}

bool Vector::isShorterThan(const double& valToCompare) const{
  return (this->squared() < (valToCompare*valToCompare)); 
}

bool Vector::isSameLength(const Vector& vecToCompare) const{
  return (this->squared() == vecToCompare.squared()); 
}

bool Vector::isSameLength(const double& valToCompare) const{
  return (this->squared() == (valToCompare*valToCompare)); 
}

bool Vector::operator>(const Vector& vec){return this->isLongerThan(vec);}
bool Vector::operator>(const double& val){return this->isLongerThan(val);}
bool Vector::operator>=(const Vector& vec){return !this->isShorterThan(vec);}
bool Vector::operator>=(const double& val){return !this->isShorterThan(val);}
bool Vector::operator<(const Vector& vec){return this->isShorterThan(vec);}
bool Vector::operator<(const double& val){return this->isShorterThan(val);}
bool Vector::operator<=(const Vector& vec){return !this->isLongerThan(vec);}
bool Vector::operator<=(const double& val){return !this->isLongerThan(val);}
bool Vector::operator==(const Vector& vec){return this->isSameLength(vec);}
bool Vector::operator==(const double& val){return this->isSameLength(val);}
bool Vector::operator!=(const Vector& vec){return !this->isSameLength(vec);}
bool Vector::operator!=(const double& val){return !this->isSameLength(val);}

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

Vector scale(Vector vec, const double& scalar){
  for (char i = 0; i<3; i++){
    vec.setCoordinate(i, vec.getCoordinate(i) * scalar);
  } 
  return vec;
}

Vector scale(const double& scalar, const Vector& vec){return scale(vec,scalar);}

Vector operator*(const Vector& vec, const double& scalar){return scale(vec, scalar);}
Vector operator*(const double& scalar, const Vector& vec){return scale(vec, scalar);}
Vector operator/(const Vector& vec, const double& div){return vec*(1/div);}

double dotproduct(const Vector& rhs, const Vector& lhs){
  double retval = 0;
  for (char i = 0; i<3; i++){
    retval += rhs.getCoordinate(i) * lhs.getCoordinate(i);
  }
  return retval;
}

double operator*(const Vector& rhs, const Vector& lhs){return dotproduct(lhs, rhs);}
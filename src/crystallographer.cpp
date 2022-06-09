#include "crystallographer.h"
#include <math.h>
#include <cmath>

// 1) Find orthogonal unit cell matrix, i.e., express the unit cell axes in cartesian coordinates
typename Cryst::MatR3 Cryst::orthogonalizeUnitCell(const std::array<double,6>& cell_param){
  /*
  Formulae to find the cartesian coordinates of axes A, B, C:
  Ax , Ay , Az
  Bx , By , Bz
  Cx , Cy , Cz
  =
  A , 0 , 0
  B*cos(gamma) , B*sin(gamma) , 0
  C*cos(beta) , C*((cos(alpha)*sin(gamma))+((cos(beta)-(cos(alpha)*cos(gamma)))*sin(gamma-90)/cos(gamma-90))) , Sqrt(C^2-Cx^2-Cy^2)

  Notes:
  alpha: angle between B and C
  beta: angle between A and C
  gamma: angle between A and B

  The origin of ABC and xyz is 0,0,0
  A is always along cartesian x axis
  B is always on the cartesian xy plane with a positive y component (By > 0)
  C is the only axis containing a cartesian z component (Cz > 0)
  */
  constexpr double PI = 3.14159265358979323846;
  constexpr double TO_RAD = PI/180;

  MatR3 cart_matrix;
  for (size_t i = 0; i < 3; ++i){
    for (size_t j = 0; j < 3; ++j){
      cart_matrix[i][j] = 0;
    }
  }

  // A is always along x axis => Ax = X
  cart_matrix[0][0] = cell_param[0];
  
  // Convert angles from degree to radian
  double gamma = cell_param[5] * TO_RAD;
  
  // Avoid floating point issues, if beta is 90 degrees
  cart_matrix[1][0] = cell_param[1] * (cell_param[5] == 90? 0 : std::cos(gamma));
  cart_matrix[1][1] = cell_param[1] * (cell_param[5] == 90? 1 : std::sin(gamma));

  // Avoid floating point issues, if alpha and gamma are 90 degrees
  if(cell_param[3] == 90 && cell_param[4] == 90 ){
    cart_matrix[2][2] = cell_param[2];
  }
  else{
    // Convert angles from degree to radian
    double alpha = cell_param[3] * TO_RAD;
    double beta  = cell_param[4] * TO_RAD;

    cart_matrix[2][0] = cell_param[2] * std::cos(beta);
    cart_matrix[2][1] = cell_param[2] * ((std::cos(alpha) * std::sin(gamma)) + 
        ((std::cos(beta) - (std::cos(alpha) * std::cos(gamma))) * std::sin(gamma-(PI/2)) / std::cos(gamma-(PI/2))));
    cart_matrix[2][2] = std::sqrt(pow(cell_param[2],2)-pow(cart_matrix[2][0],2)-pow(cart_matrix[2][1],2));
  }
  return cart_matrix;
}


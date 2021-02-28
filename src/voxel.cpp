#include "voxel.h"
#include "misc.h"
#include "atom.h"
#include <cmath> // abs, pow
#include <cassert>
#include <unordered_map>

///////////////////
// AUX FUNCTIONS //
///////////////////

void breakpoint(){return;}
inline double Voxel::calcRadiusOfInfluence(const double& max_depth){
  return max_depth != 0 ? 0.86602540378 * s_grid_size * (pow(2,max_depth) - 1) : 0;
}

bool allAtomsClose(const double&, const std::array<double,4>&, const std::array<Vector,4>&, char);

bool isInsideTetrahedron(const Vector&, const std::array<Vector,4>&, const std::array<Vector,4>&, bool&);
bool isInsideTetrahedron(const Vector&, const std::array<Vector,4>&, const std::array<Vector,4>&);
bool isInsideTetrahedron(const Vector&, const std::array<Vector,4>&, bool&);
bool isInsideTetrahedron(const Vector&, const std::array<Vector,4>&);
std::array<Vector,4> makeNormalsForTetrahedron(const std::array<Vector,4>&);

Vector calcProbeComponentAlong(const Vector&, const double&, const double&, const double&);
Vector calcProbeVectorInPlane(const std::array<Vector,4>, const std::array<double,4>&, const double&);
Vector calcProbeVectorNormal(const std::array<Vector,4>, const std::array<double,4>&, const double&, const Vector&);

bool isPointBetween(const Vector& vec_point, const Vector& vec_bounds);

/////////////////
// CONSTRUCTOR //
/////////////////

Voxel::Voxel(){
  type = 0;
}

////////////
// ACCESS //
////////////

Voxel& Voxel::getSubvoxel(const short& x, const short& y, const short& z){
  assert(x*y*z < 8);
  return data[4 * z + 2 * y + x];
}

Voxel& Voxel::getSubvoxel(const short& i){
  assert(i <= 8);
  return data[i];
}

bool Voxel::hasSubvoxel(){
  return !data.empty();
}

char Voxel::getType(){
  return type;
}

void Voxel::setType(char input){
  type = input;
}

//////////////
// SET TYPE //
//////////////

// function to call before beginning the type assignment routine in order to prepare static variables
void Voxel::storeUniversal(AtomTree atomtree, double grid_size, double r_probe1, int max_depth){
  s_atomtree = atomtree;
  s_grid_size = grid_size;
  s_r_probe1 = r_probe1;
  s_pair_data.clear();
  s_triplet_data.clear();
}

// part of the type assigment routine. first evaluation is only concerned with the relation between 
// voxels and atoms
char Voxel::evalRelationToAtoms(Vector pos_vxl, const int max_depth){
  double rad_vxl = calcRadiusOfInfluence(max_depth); // calculated every time, since max_depth may change (not expensive) 
 
  traverseTree(s_atomtree.getRoot(), s_atomtree.getMaxRad(), pos_vxl, rad_vxl, s_r_probe1, max_depth);

  if (type == 0){
    setBitOn(type,0); // is assigned
    setBitOn(type,3); // probe core
  }
  if (readBit(type,7)){splitVoxel(pos_vxl, max_depth);} // split if type mixed

  return type;
}

// adds an array of size 8 to the voxel that contains 8 subvoxels and evaluates each subvoxel's type
void Voxel::splitVoxel(const Vector& vxl_pos, const double& max_depth){
  // split into 8 subvoxels
  Vector factors;
  for(int i = 0; i < 8; i++){
    data.push_back(Voxel());
    // modify position
    factors = {
      (i%2) >= 1 ? 1 : -1,
      (i%4) >= 2 ? 1 : -1,
       i    >= 4 ? 1 : -1};

    Vector new_pos = vxl_pos + factors * s_grid_size * std::pow(2,max_depth-2);
    
    data[i].evalRelationToAtoms(new_pos, max_depth-1);
  }
}

void Voxel::traverseTree
  (const AtomNode* node, 
   const double& rad_max, 
   const Vector& pos_vxl, 
   const double& rad_vxl, 
   const double& rad_probe, 
   const int& max_depth,
   const char exit_type,
   const char dim){

  if (node == NULL || readBit(type,0)){return;}
  const Atom& atom = node->getAtom();

  // distance between atom and voxel along one dimension
  double dist1D = distance(atom.getPosVec(), pos_vxl, dim);
 
  if (abs(dist1D) > (rad_vxl + rad_max + rad_probe)){ // then atom is too far to matter for voxel type
      traverseTree(dist1D < 0 ? node->left_child : node->right_child,
          rad_max, pos_vxl, rad_vxl, rad_probe, max_depth, exit_type, (dim+1)%3);
  } 
  else{ // then atom is close enough to influence voxel type
    if(isAtom(atom, pos_vxl, rad_vxl, rad_probe)){return;}
    
//    if (type==exit_type){return;}

    // continue with both children
    for (AtomNode* child : {node->left_child, node->right_child}){
      traverseTree(child, rad_max, pos_vxl, rad_vxl, rad_probe, max_depth, exit_type, (dim+1)%3);
    }
  }
}

// go through a tree, starting from node. return a list of atoms that are a specified max distance
// from a point with radius rad_point.
void Voxel::listFromTree(
  std::vector<int>& atom_id_list,
  const AtomNode* node,
  const Vector& pos_point,
  const double& rad_point,
  const double& rad_max, // ideally this value should be retrieved from node
  const double& max_dist,
  const char dim)
{
  if (node == NULL){return;}

  // distance between atom and point along one dimension
  double dist1D = distance(node->getAtom().getPosVec(), pos_point, dim);
  double rad_atom = node->getAtom().getRad();

  if (abs(dist1D) > rad_point + rad_max + max_dist) { // then atom is too far
      listFromTree(atom_id_list, dist1D<0? node->left_child : node->right_child, pos_point, rad_point, rad_max, max_dist, (dim+1)%3);
  }
  else { // then atom may be close enough
    if ((pos_point-node->getAtom().getPosVec()) < rad_point + rad_atom + max_dist){
      atom_id_list.push_back(node->getAtomId());
    }

    // continue with both children
    for (AtomNode* child : {node->left_child, node->right_child}){
      listFromTree(atom_id_list, child, pos_point, rad_point, rad_max, max_dist, (dim+1)%3);
    }
  }
}

////////////////////
// CHECK FOR TYPE //
////////////////////

// assign a type based on the distance between a voxel and an atom
bool Voxel::isAtom(const Atom& atom, const Vector& pos_vxl, const double rad_vxl, const double rad_probe){
  Vector dist = pos_vxl - atom.getPosVec();
  if(dist < atom.getRad() - rad_vxl){
    type = 0;
    setBitOn(type,1); // inside atom
    setBitOn(type,0); // assigned
    return true;
  }
  else if (dist < atom.getRad() + rad_vxl){
    if (readBit(type,1)){return false;} // if inside atom
    type = 0;
    setBitOn(type,1); // potentially inside atom
    setBitOn(type,7); // mixed
  }
  else if (dist < atom.getRad() + rad_probe - rad_vxl){
    if (readBit(type,1)){return false;} // if mixed or inside atom
    type = 0;
    setBitOn(type,4); // potential shell
  } 
  else if (dist < atom.getRad() + rad_probe + rad_vxl){
    if (readBit(type,4) || readBit(type,1)){return false;} // if mixed, inside atom, or potential shell
    type = 0;
    setBitOn(type,4); // potentially shell
    setBitOn(type,7); // mixed
  }
  return false;
}

bool Voxel::isProbeExcluded(const Vector& vxl_pos, const double& r_probe, const double& radius_of_influence, const std::vector<int>& close_atom_ids){

  if(type == 'm'){return false;} // type 'm' can never be changed by probe

  for (int i = 0; i < close_atom_ids.size(); i++){
    Atom atom1 = AtomNode::getAtom(close_atom_ids[i]);
    // for simplicity all vectors are shifted by -vec_offset, so that atom1 is in the origin
    Vector vec_offset = Vector(atom1.getPos());

    std::array<double,4> atom_radii;
    std::array<Vector,4> vectors;

    atom_radii[0] = atom1.getRad();
    Vector vec_vxl = vxl_pos - vec_offset;

    for (int j = i+1; j < close_atom_ids.size(); j++){
      Atom atom2 = AtomNode::getAtom(close_atom_ids[j]);
      atom_radii[1] = atom2.getRad();
      vectors[1] = Vector(atom2.getPos()) - vec_offset;

      int pair_id = close_atom_ids[i] + AtomNode::getAtomList().size()*close_atom_ids[j];

      if (!allAtomsClose(r_probe, atom_radii, vectors, 2)){continue;}
      if (isExcludedByPair(vec_vxl, vectors[1], atom_radii[0], atom_radii[1], r_probe, radius_of_influence, pair_id)){return true;}

      for (int k = j+1; k < close_atom_ids.size(); k++){
        Atom atom3 = AtomNode::getAtom(close_atom_ids[k]);
        atom_radii[2] = atom3.getRad();
        vectors[2] = Vector(atom3.getPos()) - vec_offset;

//        std::array<unsigned long long int, 4> triplet_ids;
//        triplet_ids[0] = close_atom_ids[i] + AtomNode::getAtomList().size()*close_atom_ids[j] + pow(AtomNode::getAtomList().size(),2)*close_atom_ids[k];
        unsigned long long int triplet_id = close_atom_ids[i] + AtomNode::getAtomList().size()*close_atom_ids[j] + pow(AtomNode::getAtomList().size(),2)*close_atom_ids[k];

        if (!allAtomsClose(r_probe, atom_radii, vectors, 3)){continue;}
        if (isExcludedByTriplet(vec_vxl, radius_of_influence, vectors, atom_radii, r_probe, triplet_id)){return true;}

        for (int l = k+1; l < close_atom_ids.size(); l++){


          Atom atom4 = AtomNode::getAtom(close_atom_ids[l]);
          atom_radii[3] = atom4.getRad();
          vectors[3] = Vector(atom4.getPos()) - vec_offset;

          if (!allAtomsClose(r_probe, atom_radii, vectors, 4)){continue;}
          if (isExcludedByQuadruplet(vec_vxl, radius_of_influence, vectors, atom_radii, r_probe, close_atom_ids)){return true;}
        }
      }
    }
  }
  return false;
}

bool Voxel::isExcludedByQuadruplet(
    const Vector& vec_vxl,
    const double& rad_vxl,
    const std::array<Vector,4>& vec_atoms,
    const std::array<double,4>& rad_atoms,
    const double& rad_probe,
    const std::vector<int>& close_atom_ids)
{
  bool sign;
  if (!isInsideTetrahedron(vec_vxl, vec_atoms, sign)){return false;}
  setType('x'); // all voxels inside tetrahedron are assumed inaccessible
  // currently, access evaluation functions work with arrays of size four even when not strictly needed,
  // in order to avoid the use of pointers
  // this loop goes over all combinations of three atoms, i.e., all planes that make up the tetrahedron
  std::array<Vector,4> vec_plane_vertices;
  std::array<double,4> rad_plane_vertices;
  for (char i = 0; i < 4; i++){
    rad_plane_vertices[0] = rad_atoms[i];
    for (char j = i+1; j < 4; j++){
      vec_plane_vertices[1] = vec_atoms[j]-vec_atoms[i];
      rad_plane_vertices[1] = rad_atoms[j];
      for (char k = j+1; k < 4; k++){
        vec_plane_vertices[2] = vec_atoms[k]-vec_atoms[i];
        rad_plane_vertices[2] = rad_atoms[k];

//        unsigned long long int triplet_id = close_atom_ids[i] + AtomNode::getAtomList().size()*close_atom_ids[j] + pow(AtomNode::getAtomList().size(),2)*close_atom_ids[k];

        // introducing a dummy voxel as a hack, in order to independantly evaluate the voxel accessibilty
        // for every plane
        Voxel dummy = Voxel();
        dummy.isExcludedByTriplet(vec_vxl-vec_atoms[i], rad_vxl, vec_plane_vertices, rad_plane_vertices, rad_probe, 0, true);
        char type_from_plane = dummy.getType();

        // if any voxel is found to be completely accessible, then set type and end
        if (type_from_plane=='e'){
          setType('e');
          return false;
        }
        // if any voxel is found to be partially accessible, then set type and continue
        else if (type_from_plane=='m'){
          setType('m');
        }
      }
    }
  }
  // if voxel has not been determined accessible, then it is inaccessible
  return true;
}

bool Voxel::isExcludedByTriplet(
  const Vector& vec_vxl,
  const double& rad_vxl,
  const std::array<Vector,4>& vec_atom,
  const std::array<double,4>& rad_atom,
  const double& rad_probe,
  const unsigned long long int triplet_id,
  const bool side_restr)
{
  // check if between atom1 and atom2 - done by isExcludedByPair

  if (!isPointBetween(vec_vxl, vec_atom[2])){return false;} // filter voxels outside range

  if (triplet_id == 0 || s_triplet_data.find(triplet_id) == s_triplet_data.end()){

    Vector vec_probe_plane = calcProbeVectorInPlane(vec_atom, rad_atom, rad_probe);

    Vector vec_probe_normal = calcProbeVectorNormal(vec_atom, rad_atom, rad_probe, vec_probe_plane);

    s_triplet_data[triplet_id] = TripletBundle(vec_probe_plane, vec_probe_normal);
  }

  Vector vec_probe;

  // side restricted mode: the probe always sits on the other side of the plane relative to
  // the voxel. we do not check whether the voxel is inside the tetrahedron, because it never is
  if (side_restr){
    vec_probe = s_triplet_data[triplet_id].vec_probe_plane
      + s_triplet_data[triplet_id].vec_probe_normal * ( std::signbit(s_triplet_data[triplet_id].vec_probe_normal*vec_vxl)? 1 : -1 );
  }

  // default mode: the probe always sits on the same side as the voxel
  else {
    vec_probe = s_triplet_data[triplet_id].vec_probe_plane
      + s_triplet_data[triplet_id].vec_probe_normal * ( std::signbit(s_triplet_data[triplet_id].vec_probe_normal*vec_vxl)? -1 : 1 );

    // check whether vxl is inside tetrahedron spanned by atoms1-3 and probe
    std::array<Vector,4> vec_vertices; // vectors pointing to vertices of the tetrahedron
    {
      for (char i = 0; i<3; i++){
        vec_vertices[i] = vec_atom[i];
      }
      vec_vertices[3] = vec_probe;
    }
    if (!isInsideTetrahedron(vec_vxl, vec_vertices)){return false;} // stop, if not inside tetrahedron
  }

  return isExcludedSetType(vec_vxl, rad_vxl, vec_probe, rad_probe);
}

bool Voxel::isExcludedByPair(
    const Vector& vec_vxl,
    const Vector& vec_atat,
    const double& rad_atom1,
    const double& rad_atom2,
    const double& rad_probe,
    const double& rad_vxl,
    int pair_id)
{

  Vector unitvec_parallel = vec_atat.normalise();
  double vxl_parallel = vec_vxl * unitvec_parallel;

  if (s_pair_data.find(pair_id) == s_pair_data.end()){

    // TODO: consider making a function that produces vec_probe and takes either PairBundle or values as args
    double dist_atom1_probe = rad_atom1 + rad_probe;
    double dist_atom2_probe = rad_atom2 + rad_probe;
    double dist_atom1_atom2 = vec_atat.length();

    double probe_parallel = ((pow(dist_atom1_probe,2) + pow(dist_atom1_atom2,2) - pow(dist_atom2_probe,2))/(2*dist_atom1_atom2));
    double probe_orthogonal = pow(pow(dist_atom1_probe,2)-pow(probe_parallel,2),0.5);

    s_pair_data[pair_id] = PairBundle(unitvec_parallel, probe_parallel, probe_orthogonal);
  }
  Vector unitvec_orthogonal = (vec_vxl-s_pair_data[pair_id].unitvec_parallel*vxl_parallel).normalise();

  Vector vec_probe = s_pair_data[pair_id].probe_parallel * unitvec_parallel + s_pair_data[pair_id].probe_orthogonal * unitvec_orthogonal;

  if (vec_vxl.isInsideTriangle({Vector(), vec_atat, vec_probe})){
    return isExcludedSetType(vec_vxl, rad_vxl, vec_probe, rad_probe);
  }
  return false;
}

bool Voxel::isExcludedSetType(const Vector& vec_vxl, const double& rad_vxl, const Vector& vec_probe, const double& rad_probe){
  if (vec_probe-vec_vxl > (rad_probe+rad_vxl)){ // then all subvoxels are inaccessible
    setType('x');
    return true;
  }
  else if (vec_probe-vec_vxl <= (rad_probe-rad_vxl)){ // then all subvoxels are accessible
    return false;
  }
  else { // then each subvoxel has to be evaluated
    setType('m');
    return false;
  }
}

///////////
// TALLY //
///////////

// TODO: Optimise. Allow for tallying multiples types at once
size_t Voxel::tallyVoxelsOfType(const char volume_type, const int max_depth)
{
  // if voxel is of type "mixed" (i.e. data vector is not empty)
  if(!data.empty()){
    // then total number of voxels is given by tallying all subvoxels
    size_t total = 0;
    for(int i = 0; i < 8; i++){
      total += data[i].tallyVoxelsOfType(volume_type, max_depth-1);
    }
    return total;
  }
  // if voxel is of the type of interest, tally the voxel in units of bottom level voxels
  else if(type == volume_type){
    return pow(pow(2,max_depth),3); // return number of bottom level voxels
  }
  // if neither empty nor of the type of interest, then the voxel doesn't count towards the total
  return 0;
}

//////////////////////////////
// AUX FUNCTION DEFINITIONS //
//////////////////////////////

bool allAtomsClose(
    const double& r_probe,
    const std::array<double,4>& atom_radii,
    const std::array<Vector,4>& vectors,
    char n_atoms)
{
  bool all_atoms_close = true;

  Vector last_added = vectors[n_atoms-1];
  double rad_last_added = atom_radii[n_atoms-1];
  for (char i = 0; i < n_atoms-1; i++){
    if (vectors[i]-last_added > 2*r_probe + atom_radii[i] + rad_last_added){
      return false;
    }
  }
  return true; // if all atoms close
}

bool isInsideTetrahedron(
    const Vector& vec_point,
    const std::array<Vector,4>& vec_vertices,
    const std::array<Vector,4>& norm_planes,
    bool& sign)
{
  // when multiplying the voxel vector with a normal belonging to a plane, the
  // sign of the resulting vector holds information about which side of the plane
  // the voxel is on. The order of normals in norm_planes is integral to the
  // success of this method!
  // if all signs are the same, then the voxel is inside the tetrahedron
  for (int i = 0; i < 4; i++){
    if (!i){
      sign = std::signbit((vec_point-vec_vertices[i])*norm_planes[i]);
    }
    else if (sign != std::signbit((vec_point-vec_vertices[i])*norm_planes[i])){
      return false;
    }
  }
  return true;
}

bool isInsideTetrahedron(
    const Vector& vec_point,
    const std::array<Vector,4>& vec_vertices,
    const std::array<Vector,4>& norm_planes)
{
  bool dummy = false;
  return isInsideTetrahedron(vec_point, vec_vertices, norm_planes, dummy);
}

bool isInsideTetrahedron(
    const Vector& vec_point,
    const std::array<Vector,4>& vec_vertices,
  bool& sign)
{
  return isInsideTetrahedron(vec_point, vec_vertices, makeNormalsForTetrahedron(vec_vertices), sign);
}

bool isInsideTetrahedron(
    const Vector& vec_point,
    const std::array<Vector,4>& vec_vertices)
{
  return isInsideTetrahedron(vec_point, vec_vertices, makeNormalsForTetrahedron(vec_vertices));
}

std::array<Vector,4> makeNormalsForTetrahedron(const std::array<Vector,4>& vec_vertices)
{
  std::array<Vector,4> norm_planes;
  char i = 0;
  for (char j = 0; j < 4; j++){
    for (char k = j+1; k < 4; k++){
      for (char l = k+1; l < 4; l++){
        norm_planes[i] = pow(-1,i) * crossproduct(vec_vertices[k]-vec_vertices[j], vec_vertices[l]-vec_vertices[j]);
        i++;
      }
    }
  }
  return norm_planes;
}

bool isPointBetween(const Vector& vec_point, const Vector& vec_bounds){
  Vector unitvec_bounds = vec_bounds.normalise();
  double dist_point_along_bounds = unitvec_bounds * vec_point;
  return (dist_point_along_bounds > 0 && vec_bounds > dist_point_along_bounds);
}

Vector calcProbeComponentAlong(const Vector& vec_atom_atom, const double& rad_atom_origin, const double& rad_atom_destination, const double& rad_probe)
{
  double dist = vec_atom_atom.length();
  double dist_probe = (rad_atom_origin-rad_atom_destination)*(((rad_atom_origin + rad_atom_destination) + 2*rad_probe)/(2*dist)) + (dist/2);
  return dist_probe * vec_atom_atom.normalise(); // the unitvector here may be calculated multiple times
}

Vector calcProbeVectorInPlane(const std::array<Vector,4> vec_atom, const std::array<double,4>& rad_atom, const double& rad_probe){

  Vector vec_probe_12 = calcProbeComponentAlong(vec_atom[1], rad_atom[0], rad_atom[1], rad_probe);
  Vector vec_probe_13 = calcProbeComponentAlong(vec_atom[2], rad_atom[0], rad_atom[2], rad_probe);

  Vector vec_normal_12 = crossproduct(crossproduct(vec_probe_12,vec_probe_13),vec_probe_12);
  Vector vec_normal_13 = crossproduct(crossproduct(vec_probe_12,vec_probe_13),vec_probe_13);

  double c1 = ((vec_probe_13[1]-vec_probe_12[1]) + (vec_normal_13[1]/vec_normal_13[0])*(vec_probe_12[0]-vec_probe_13[0]))/(vec_normal_12[1]-(vec_normal_12[0]*vec_normal_13[1]/vec_normal_13[0])); // system of linear equations

  return vec_probe_12 + (c1 * vec_normal_12);
}

Vector calcProbeVectorNormal(const std::array<Vector,4> vec_atom, const std::array<double,4>& rad_atom, const double& rad_probe, const Vector& vec_probe_plane){

  Vector unitvec_normal = crossproduct(vec_atom[1],vec_atom[2]).normalise();

  return unitvec_normal * pow((pow(rad_atom[0]+rad_probe,2) - vec_probe_plane*vec_probe_plane),0.5);
}

/////////////////
// OUTPUT TYPE //
/////////////////

void Voxel::fillTypeTensor(
    Container3D<char>& type_tensor, 
    const std::array<unsigned long int,3> block_start, 
    const int remaining_depth){
  if (remaining_depth == 0){
    type_tensor.getElement(block_start) = type;
  }
  else if (data.empty()){
    for (unsigned int i = block_start[0]; i < block_start[0]+pow(2,remaining_depth); i++){
      for (unsigned int j = block_start[1]; j < block_start[1]+pow(2,remaining_depth); j++){
        for (unsigned int k = block_start[2]; k < block_start[2]+pow(2,remaining_depth); k++){
          type_tensor.getElement(i,j,k) = type;
        }
      }
    }
  }
  else {
    std::array<unsigned long int,3> new_start;
    for (char i = 0; i < 2; i++){
      new_start[0] = block_start[0] + i*pow(2,remaining_depth-1);
      for (char j = 0; j < 2; j++){
        new_start[1] = block_start[1] + j*pow(2,remaining_depth-1);
        for (char k = 0; k < 2; k++){
          new_start[2] = block_start[2] + k*pow(2,remaining_depth-1);

          getSubvoxel(i,j,k).fillTypeTensor(type_tensor, new_start, remaining_depth-1);
        }
      }
    }
  }
}

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#  include <wx/wx.h>
#endif

#include "base.h"
#include "controller.h"
#include "misc.h"
#include "flags.h"
#include <cassert>
#include <filesystem>
#include <sstream>

// contains all command line options
static const wxCmdLineEntryDesc s_cmd_line_desc[] =
{
  // required
  { wxCMD_LINE_OPTION, "r", "radius", "Probe radius", wxCMD_LINE_VAL_DOUBLE},
  { wxCMD_LINE_OPTION, "g", "grid", "Spatial resolution of the underlying grid", wxCMD_LINE_VAL_DOUBLE},
  { wxCMD_LINE_OPTION, "fs", "file-structure", "Path to the structure file", wxCMD_LINE_VAL_STRING},
  // optional
  { wxCMD_LINE_OPTION, "fr", "file-radius", "Path to the radius file", wxCMD_LINE_VAL_STRING},
  { wxCMD_LINE_OPTION, "do", "dir-output", "Path to the output directory", wxCMD_LINE_VAL_STRING},
  { wxCMD_LINE_OPTION, "o", "output", "Limit output to values", wxCMD_LINE_VAL_STRING},
  { wxCMD_LINE_OPTION, "r2", "radius2", "Large probe radius", wxCMD_LINE_VAL_DOUBLE},
  { wxCMD_LINE_OPTION, "d", "depth", "Octree depth", wxCMD_LINE_VAL_NUMBER},
  { wxCMD_LINE_SWITCH, "ht", "hetatm", "Include HETATM from pdb file", wxCMD_LINE_VAL_NONE, 0},
  { wxCMD_LINE_SWITCH, "uc", "unitcell", "Evaluate unit cell", wxCMD_LINE_VAL_NONE, 0},
  { wxCMD_LINE_SWITCH, "sf", "surface", "Calculate surfaces", wxCMD_LINE_VAL_NONE, 0},
  { wxCMD_LINE_SWITCH, "xr", "export-report", "Export report", wxCMD_LINE_VAL_NONE, 0},
  { wxCMD_LINE_SWITCH, "xt", "export-total", "Export total surface map", wxCMD_LINE_VAL_NONE, 0},
  { wxCMD_LINE_SWITCH, "xc", "export-cavities", "Export surface maps for all cavities", wxCMD_LINE_VAL_NONE, 0},
  { wxCMD_LINE_SWITCH, "q", "quiet", "Silence progress reporting", wxCMD_LINE_VAL_NONE, 0},
  { wxCMD_LINE_OPTION, "u", "unittest", "Run a programmed unit test", wxCMD_LINE_VAL_STRING},
  { wxCMD_LINE_SWITCH, "v", "version", "Display the app version", wxCMD_LINE_VAL_NONE},
  { wxCMD_LINE_NONE }
};

static const std::vector<std::string> s_required_args = {"r", "g", "fs"};

bool validateProbes(const double, const double, const bool);
bool validateExport(const std::string, const std::vector<bool>);
bool validateOutput(const std::string);
bool validatePdb(const std::string, const bool, const bool);

// return true to supress GUI, return false to open GUI
void MainApp::evalCmdLine(){
  // if there are no cmd line arguments, open app normally
  silenceGUI(true);
  wxCmdLineParser parser = wxCmdLineParser(argc,argv);
  parser.SetDesc(s_cmd_line_desc);
  // if something is wrong with the cmd line args, stop
  if(parser.Parse() != 0){return;}
  // version
  if(parser.Found("v")){
    Ctrl::getInstance()->version();
    return;
  }
  // unit tests
  wxString unittest_id;
  if (parser.Found("u",&unittest_id)){
    std::cout << "Selected unit test: " << unittest_id << std::endl;
    if (unittest_id=="excluded"){
      Ctrl::getInstance()->unittestExcluded();
    }
    else if (unittest_id=="protein"){
      Ctrl::getInstance()->unittestProtein();
    }
    else if (unittest_id=="radius"){
      Ctrl::getInstance()->unittestRadius();
    }
    else if (unittest_id=="2probe"){
      Ctrl::getInstance()->unittest2Probe();
    }
    else if (unittest_id=="surface"){
      Ctrl::getInstance()->unittestSurface();
    }
    else if (unittest_id=="floodfill"){
      Ctrl::getInstance()->unittestFloodfill();
    }
    else {
      std::cout << "Invalid selection" << std::endl;
    }
    return;
  }
  // check if all required arguments are available
  for (auto& arg_name : s_required_args){
    if (!parser.Found(arg_name)){
      Ctrl::getInstance()->displayErrorMessage(901);
      return;
    }
  }

  Ctrl::getInstance()->hush(parser.Found("q"));

  // minimum required arguments for calculation
  double probe_radius_s;
  double grid_resolution;
  wxString structure_file_path;

  parser.Found("r",&probe_radius_s);
  parser.Found("g",&grid_resolution);
  parser.Found("fs",&structure_file_path);
 
  // optional arguments with default values
  wxString radius_file_path = getResourcesDir() + "/radii.txt";
  wxString output_dir_path = "";
  wxString output = "all"; // TODO: allow partial output (for instance: "vol", "surf", "time", etc.)
  double probe_radius_l = 0;
  long tree_depth = 4;
  bool opt_include_hetatm = false;
  bool opt_unit_cell = false;
  bool opt_surface_area = false;
  bool opt_probe_mode = false;
  bool exp_report = false;
  bool exp_total_map = false;
  bool exp_cavity_maps = false;

  parser.Found("fr",&radius_file_path);
  parser.Found("do",&output_dir_path);
  parser.Found("o",&output);
  parser.Found("r2",&probe_radius_l);
  parser.Found("d",&tree_depth);
  opt_include_hetatm = parser.Found("ht");
  opt_unit_cell = parser.Found("uc");
  opt_surface_area = parser.Found("sf");
  opt_probe_mode = parser.Found("r") && parser.Found("r2");
  exp_report = parser.Found("xr");
  exp_total_map = parser.Found("xt");
  exp_cavity_maps = parser.Found("xc");

  if(!validateProbes(probe_radius_s, probe_radius_l, opt_probe_mode)
      || !validateExport(output_dir_path.ToStdString(), {exp_report, exp_total_map, exp_cavity_maps})
      || !validateOutput(output.ToStdString())
      || !validatePdb(structure_file_path.ToStdString(), opt_include_hetatm, opt_unit_cell)){
    return;
  }

  // run calculation
  Ctrl::getInstance()->runCalculation(
      probe_radius_s, 
      probe_radius_l, 
      grid_resolution, 
      structure_file_path.ToStdString(), 
      radius_file_path.ToStdString(),
      output_dir_path.ToStdString(),
      (int)tree_depth,
      opt_include_hetatm,
      opt_unit_cell,
      opt_surface_area,
      opt_probe_mode,
      exp_report,
      exp_total_map,
      exp_cavity_maps);
}

bool validateProbes(const double r1, const double r2, const bool pm){
  if(pm && r2 < r1){
    Ctrl::getInstance()->displayErrorMessage(104);
    return false;
  }
  return true;
}

bool validateExport(const std::string out_dir, const std::vector<bool> exp_options){
  bool any_option_on = isIncluded(true,exp_options);
  if (any_option_on && !std::filesystem::is_directory(std::filesystem::path(out_dir))){
    Ctrl::getInstance()->displayErrorMessage(302);
    return false;
  }
  return true;
}

bool validateOutput(const std::string output){ 
  std::stringstream ss(output);
  std::vector<std::string> options;
  while(ss.good()){
    std::string substr;
    getline(ss, substr, ',');
    options.push_back(substr);
  }
  return true;
}

bool validatePdb(const std::string file, const bool hetatm, const bool unitcell){
  if (fileExtension(file) != "pdb" && (hetatm || unitcell)){
    Ctrl::getInstance()->displayErrorMessage(115);
    return false; 
  }
  return true;
}

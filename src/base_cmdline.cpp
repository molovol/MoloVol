#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iostream>
#include <optional>
#include "controller.h"
#include "misc.h"
#include "special_chars.h"

struct CommandLineOption {
  std::string short_name;
  std::string long_name;
  std::string description;
  bool is_switch;  // true for flags, false for options with values
  bool is_required;
};

class CommandLineParser {
private:
  const std::vector<CommandLineOption> options = {
    {"h", "help", "Display help for command line interface", true, false},
    {"r", "radius", "Probe radius", false, true},
    {"g", "grid", "Spatial resolution of the underlying grid", false, true},
    {"fs", "file-structure", "Path to the structure file", false, true},
    {"fe", "file-elements", "Path to the elements file", false, false},
    {"do", "dir-output", "Path to the output directory", false, false},
    {"r2", "radius2", "Large probe radius (for two-probe mode)", false, false},
    {"d", "depth", "Octree depth", false, false},
    {"ht", "hetatm", "Include HETATM from pdb file", true, false},
    {"uc", "unitcell", "Evaluate unit cell", true, false},
    {"sf", "surface", "Calculate surfaces", true, false},
    {"xr", "export-report", "Export report (requires:-do)", true, false},
    {"xt", "export-total", "Export total surface map (requires:-do)", true, false},
    {"xc", "export-cavities", "Export surface maps for all cavities (requires:-do)", true, false},
    {"o", "output", "Control what parts of the output to display (default:all)", false, false},
    {"q", "quiet", "Silence progress reporting", true, false},
    {"un", "unicode", "Allow unicode output", true, false},
    {"v", "version", "Display the app version", true, false}
  };

  std::map<std::string, std::string> _parsed_options;
  std::map<std::string, bool> _parsed_flags;

public:
  bool parse(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
      std::string arg = argv[i];
      if (arg[0] != '-') continue;

      if (arg[1] == '-') {
        // Long option
        auto eq_pos = arg.find('=');
        std::string name = (eq_pos != std::string::npos) ? arg.substr(2, eq_pos - 2) : arg.substr(2);
        
        auto opt_it = std::find_if(options.begin(), options.end(),
          [&name](const CommandLineOption& opt) { return opt.long_name == name; });
        
        if (opt_it == options.end()) {
          std::cerr << "Unknown option: " << arg << std::endl;
          return false;
        }

        if (opt_it->is_switch) {
          _parsed_flags[name] = true;
        } else {
          std::string value;
          if (eq_pos != std::string::npos) {
            value = arg.substr(eq_pos + 1);
          } else if (i + 1 < argc) {
            value = argv[++i];
          } else {
            std::cerr << "Option " << arg << " requires a value" << std::endl;
            return false;
          }
          _parsed_options[name] = value;
        }
      } else {
        // Short option
        std::string name = arg.substr(1);
        
        auto opt_it = std::find_if(options.begin(), options.end(),
          [&name](const CommandLineOption& opt) { return opt.short_name == name; });
        
        if (opt_it == options.end()) {
          std::cerr << "Unknown option: " << arg << std::endl;
          return false;
        }

        if (opt_it->is_switch) {
          _parsed_flags[opt_it->long_name] = true;
        } else {
          if (i + 1 < argc) {
            _parsed_options[opt_it->long_name] = argv[++i];
          } else {
            std::cerr << "Option " << arg << " requires a value" << std::endl;
            return false;
          }
        }
      }
    }

    // Check required options (skip if help or version is requested)
    if (_parsed_flags.find("help") == _parsed_flags.end() && _parsed_flags.find("version") == _parsed_flags.end()) {
      for (const auto& opt : options) {
        if (opt.is_required && !opt.is_switch && _parsed_options.find(opt.long_name) == _parsed_options.end()) {
          std::cerr << "Missing required option: --" << opt.long_name << " (-" << opt.short_name << ")" << std::endl;
          return false;
        }
      }
    }

    return true;
  }

  bool found(const std::string& name) const {
    return _parsed_flags.find(name) != _parsed_flags.end();
  }

  std::optional<std::string> getValue(const std::string& name) const {
    auto it = _parsed_options.find(name);
    return (it != _parsed_options.end()) ? std::make_optional(it->second) : std::nullopt;
  }

  void displayHelp() const {
    std::cout << "Usage:\n";
    for (const auto& opt : options) {
      std::cout << "  -" << opt.short_name << ", --" << opt.long_name;
      if (opt.is_required) std::cout << " (required)";
      std::cout << "\n    " << opt.description << std::endl;
    }
  }
};

bool validateProbes(double radius_s, double radius_l, bool two_probe_mode) {
  if (radius_s <= 0) {
    Ctrl::getInstance()->displayErrorMessage(900);
    return false;
  }
  if (two_probe_mode && radius_l <= radius_s) {
    Ctrl::getInstance()->displayErrorMessage(901);
    return false;
  }
  return true;
}

bool validateExport(const std::string& output_dir, const std::vector<bool>& export_flags) {
  bool any_export = std::any_of(export_flags.begin(), export_flags.end(), [](bool flag) { return flag; });
  if (any_export && output_dir.empty()) {
    std::cerr << "Export options require an output directory (-do/--dir-output)" << std::endl;
    return false;
  }
  return true;
}

bool validatePdb(const std::string& file_path, bool include_hetatm, bool unit_cell) {
  if (unit_cell) {
    std::string ext = file_path.substr(file_path.find_last_of('.') + 1);
    if (ext != "pdb" && ext != "cif") {
      std::cerr << "Unit cell analysis requires PDB or CIF file format" << std::endl;
      return false;
    }
  }
  return true;
}

// Display options map
static const std::map<std::string, unsigned> DISPLAY_OPTIONS = {
  {"none", mvOUT_NONE},
  {"inputfile", mvOUT_STRUCTURE},
  {"resolution", mvOUT_RESOLUTION},
  {"depth", mvOUT_DEPTH},
  {"radius_small", mvOUT_RADIUS_S},
  {"radius_large", mvOUT_RADIUS_L},
  {"input", mvOUT_INP},
  {"hetatm", mvOUT_OPT_HETATM},
  {"unitcell", mvOUT_OPT_UNITCELL},
  {"probemode", mvOUT_OPT_PROBEMODE},
  {"surface", mvOUT_OPT_SURFACE},
  {"options", mvOUT_OPT},
  {"formula", mvOUT_FORMULA},
  {"time", mvOUT_TIME},
  {"vol_vdw", mvOUT_VOL_VDW},
  {"vol_inaccessible", mvOUT_VOL_INACCESSIBLE},
  {"vol_core_s", mvOUT_VOL_CORE_S},
  {"vol_shell_s", mvOUT_VOL_SHELL_S},
  {"vol_core_l", mvOUT_VOL_CORE_L},
  {"vol_shell_l", mvOUT_VOL_SHELL_L},
  {"vol_mol", mvOUT_VOL_MOL},
  {"vol", mvOUT_VOL},
  {"surf_vdw", mvOUT_SURF_VDW},
  {"surf_mol", mvOUT_SURF_MOL},
  {"surf_excluded_s", mvOUT_SURF_EXCLUDED_S},
  {"surf_accessible_s", mvOUT_SURF_ACCESSIBLE_S},
  {"surf", mvOUT_SURF},
  {"cavities", mvOUT_CAVITIES},
  {"all", mvOUT_ALL}
};

unsigned evalDisplayOptions(const std::string& output) {
  std::stringstream ss(output);
  std::string option;
  unsigned display_flag = 0;
  bool unknown_flag = false;
  
  while (std::getline(ss, option, ',')) {
    auto it = DISPLAY_OPTIONS.find(option);
    if (it != DISPLAY_OPTIONS.end()) {
      display_flag |= it->second;
    } else {
      unknown_flag = true;
    }
  }
  
  if (unknown_flag) {
    Ctrl::getInstance()->displayErrorMessage(902);
  }
  return display_flag;
}

bool evalCmdLine(int argc, char* argv[]) {
  CommandLineParser parser;
  if (!parser.parse(argc, argv)) return false;

  if (parser.found("help")) {
    parser.displayHelp();
    return true;
  }

  parser.found("unicode") ? Symbol::allow_unicode() : Symbol::limit2ascii();

  if (parser.found("version")) {
    Ctrl::getInstance()->version();
    return true;
  }

  Ctrl::getInstance()->hush(parser.found("quiet"));

  // Parse required options
  auto probe_radius_s = std::stod(parser.getValue("radius").value());
  auto grid_resolution = std::stod(parser.getValue("grid").value());
  auto structure_file_path = parser.getValue("file-structure").value();

  // Optional parameters with defaults
  auto elements_file_path = parser.getValue("file-elements").value_or(Ctrl::getDefaultElemPath());
  auto output_dir_path = parser.getValue("dir-output").value_or("");
  auto output = parser.getValue("output").value_or("all");
  auto probe_radius_l = parser.getValue("radius2") ? std::stod(parser.getValue("radius2").value()) : 0.0;
  auto tree_depth = parser.getValue("depth") ? std::stol(parser.getValue("depth").value()) : 4;

  bool opt_include_hetatm = parser.found("hetatm");
  bool opt_unit_cell = parser.found("unitcell");
  bool opt_surface_area = parser.found("surface");
  bool opt_probe_mode = parser.found("radius") && parser.found("radius2");
  bool exp_report = parser.found("export-report");
  bool exp_total_map = parser.found("export-total");
  bool exp_cavity_maps = parser.found("export-cavities");

  if (!validateProbes(probe_radius_s, probe_radius_l, opt_probe_mode) ||
      !validateExport(output_dir_path, {exp_report, exp_total_map, exp_cavity_maps}) ||
      !validatePdb(structure_file_path, opt_include_hetatm, opt_unit_cell)) {
    return false;
  }

  unsigned display_flag = evalDisplayOptions(output);

  return Ctrl::getInstance()->runCalculation(
      probe_radius_s, probe_radius_l, grid_resolution, structure_file_path,
      elements_file_path, output_dir_path, tree_depth, opt_include_hetatm,
      opt_unit_cell, opt_surface_area, opt_probe_mode, exp_report,
      exp_total_map, exp_cavity_maps, display_flag);
}

#ifndef MOLOVOL_GUI
int main(int argc, char* argv[]) {
  Ctrl::getInstance()->disableGUI();
  if (argc < 2) {
    std::cout << "Use --help for usage information\n";
    return 1;
  }
  return evalCmdLine(argc, argv) ? 0 : 1;
}
#endif
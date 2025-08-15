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

      std::string opt_name = arg.substr(arg[1] == '-' ? 2 : 1);
      auto option = findOption(opt_name);
      if (!option) {
        std::cerr << "Unknown option: " << arg << std::endl;
        return false;
      }

      if (option->is_switch) {
        _parsed_flags[option->long_name] = true;
      } else if (i + 1 < argc) {
        _parsed_options[option->long_name] = argv[++i];
      } else {
        std::cerr << "Missing value for option: " << arg << std::endl;
        return false;
      }
    }

    // Don't validate required options for help or version flags
    if (_parsed_flags.count("help") > 0 || _parsed_flags.count("version") > 0) {
      return true;
    }

    return validateRequiredOptions();
  }

  bool found(const std::string& name) const {
    return _parsed_flags.count(name) > 0 || _parsed_options.count(name) > 0;
  }

  std::optional<std::string> getValue(const std::string& name) const {
    auto it = _parsed_options.find(name);
    return it != _parsed_options.end() ? std::optional<std::string>(it->second) : std::nullopt;
  }

  void displayHelp() const {
    std::cout << "Usage:\n";
    for (const auto& opt : options) {
      std::cout << "  -" << opt.short_name << ", --" << opt.long_name
               << (opt.is_required ? " (required)" : "") << "\n    "
               << opt.description << "\n";
    }
  }

private:
  std::optional<CommandLineOption> findOption(const std::string& name) const {
    for (const auto& opt : options) {
      if (opt.short_name == name || opt.long_name == name) {
        return opt;
      }
    }
    return std::nullopt;
  }

  bool validateRequiredOptions() const {
    std::vector<std::string> missing;
    for (const auto& opt : options) {
      if (opt.is_required && !found(opt.long_name)) {
        missing.push_back(opt.long_name);
      }
    }

    if (!missing.empty()) {
      Ctrl::getInstance()->displayErrorMessage(910 + missing.size(), missing);
      return false;
    }
    return true;
  }
};

bool validateProbes(double r1, double r2, bool pm) {
  if (pm && r2 < r1) {
    Ctrl::getInstance()->displayErrorMessage(104);
    return false;
  }
  return true;
}

bool validateExport(const std::string& out_dir, const std::vector<bool>& exp_options) {
  if (isIncluded(true, exp_options) && out_dir.empty()) {
    Ctrl::getInstance()->displayErrorMessage(302);
    return false;
  }
  return true;
}

bool validatePdb(const std::string& file, bool hetatm, bool unitcell) {
  std::string extension = fileExtension(file);
  if (extension != "pdb" && extension != "cif" && (hetatm || unitcell)) {
    Ctrl::getInstance()->displayErrorMessage(115);
    return false;
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


int main(int argc, char* argv[]) {
  Ctrl::getInstance()->disableGUI();
  if (argc < 2) {
    std::cout << "Use --help for usage information\n";
    return 1;
  }
  return evalCmdLine(argc, argv) ? 0 : 1;
}

#ifndef MOLOVOL_BASE_CMDLINE_H
#define MOLOVOL_BASE_CMDLINE_H

#include <string>
#include <vector>
#include <map>
#include <optional>
#include "flags.h"

struct CommandLineOption {
    std::string shortName;
    std::string longName;
    std::string description;
    bool isSwitch;  // true for flags, false for options with values
    bool isRequired;
};

class CommandLineParser {
public:
    bool parse(int argc, char* argv[]); // Original method for native environment
    bool parse(const std::vector<std::string>& args); // New method for WASM
    bool found(const std::string& name) const;
    std::optional<std::string> getValue(const std::string& name) const;
    void displayHelp() const;
    const std::vector<CommandLineOption>& getOptions() const { return options; }

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

    std::map<std::string, std::string> parsedOptions;
    std::map<std::string, bool> parsedFlags;

    std::optional<CommandLineOption> findOption(const std::string& name) const;
    bool validateRequiredOptions() const;
};

// Validation function declarations
bool validateProbes(double r1, double r2, bool pm);
bool validateExport(const std::string& out_dir, const std::vector<bool>& exp_options);
bool validatePdb(const std::string& file, bool hetatm, bool unitcell);

// Display options constants - declare as extern
inline const std::map<std::string, unsigned> DISPLAY_OPTIONS = {
    {"none", mvOUT_NONE}, {"inputfile", mvOUT_STRUCTURE}, {"resolution", mvOUT_RESOLUTION},
    {"depth", mvOUT_DEPTH}, {"radius_small", mvOUT_RADIUS_S}, {"radius_large", mvOUT_RADIUS_L},
    {"input", mvOUT_INP}, {"hetatm", mvOUT_OPT_HETATM}, {"unitcell", mvOUT_OPT_UNITCELL},
    {"probemode", mvOUT_OPT_PROBEMODE}, {"surface", mvOUT_OPT_SURFACE}, {"options", mvOUT_OPT},
    {"formula", mvOUT_FORMULA}, {"time", mvOUT_TIME}, {"vol_vdw", mvOUT_VOL_VDW},
    {"vol_inaccessible", mvOUT_VOL_INACCESSIBLE}, {"vol_core_s", mvOUT_VOL_CORE_S},
    {"vol_shell_s", mvOUT_VOL_SHELL_S}, {"vol_core_l", mvOUT_VOL_CORE_L},
    {"vol_shell_l", mvOUT_VOL_SHELL_L}, {"vol_mol", mvOUT_VOL_MOL}, {"vol", mvOUT_VOL},
    {"surf_vdw", mvOUT_SURF_VDW}, {"surf_mol", mvOUT_SURF_MOL},
    {"surf_excluded_s", mvOUT_SURF_EXCLUDED_S}, {"surf_accessible_s", mvOUT_SURF_ACCESSIBLE_S},
    {"surf", mvOUT_SURF}, {"cavities", mvOUT_CAVITIES}, {"all", mvOUT_ALL}
};

// Display options evaluation function
unsigned evalDisplayOptions(const std::string& output);

#endif // MOLOVOL_BASE_CMDLINE_H
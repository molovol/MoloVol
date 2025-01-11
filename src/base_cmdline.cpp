#include "base_cmdline.h"
#include "controller.h"
#include "misc.h"
#include "special_chars.h"
#include <iostream>
#include <sstream>

bool CommandLineParser::parse(const std::vector<std::string>& args) {
    try {
        // Check for empty arguments
        if (args.empty()) {
            Ctrl::getInstance()->displayErrorMessage(901); // Assuming 901 is appropriate error code
            return false;
        }

        // Process arguments directly without conversion to char*
        for (size_t i = 1; i < args.size(); i++) {
            const std::string& arg = args[i];
            if (arg.empty()) {
                Ctrl::getInstance()->displayErrorMessage(901); // Empty argument
                return false;
            }

            if (arg[0] != '-') continue;

            std::string optName = arg.substr(arg[1] == '-' ? 2 : 1);
            if (optName.empty()) {
                Ctrl::getInstance()->displayErrorMessage(901); // Invalid option format
                return false;
            }

            auto option = findOption(optName);
            if (!option) {
                std::cerr << "Unknown option: " << arg << std::endl;
                return false;
            }

            if (option->isSwitch) {
                parsedFlags[option->longName] = true;
            } else if (i + 1 < args.size()) {
                const std::string& value = args[++i];
                if (value.empty() || value[0] == '-') {
                    std::cerr << "Invalid value for option: " << arg << std::endl;
                    return false;
                }
                parsedOptions[option->longName] = value;
            } else {
                std::cerr << "Missing value for option: " << arg << std::endl;
                return false;
            }
        }

        // Don't validate required options for help or version flags
        if (parsedFlags.count("help") > 0 || parsedFlags.count("version") > 0) {
            return true;
        }

        return validateRequiredOptions();
    } catch (const std::exception& e) {
        std::cerr << "Error parsing arguments: " << e.what() << std::endl;
        Ctrl::getInstance()->displayErrorMessage(901); // General parsing error
        return false;
    }
}

bool CommandLineParser::found(const std::string& name) const {
    return parsedFlags.count(name) > 0 || parsedOptions.count(name) > 0;
}

std::optional<std::string> CommandLineParser::getValue(const std::string& name) const {
    auto it = parsedOptions.find(name);
    return it != parsedOptions.end() ? std::optional<std::string>(it->second) : std::nullopt;
}

void CommandLineParser::displayHelp() const {
    std::cout << "Usage:\n";
    for (const auto& opt : options) {
        std::cout << "  -" << opt.shortName << ", --" << opt.longName
                 << (opt.isRequired ? " (required)" : "") << "\n    "
                 << opt.description << "\n";
    }
}

std::optional<CommandLineOption> CommandLineParser::findOption(const std::string& name) const {
    for (const auto& opt : options) {
        if (opt.shortName == name || opt.longName == name) {
            return opt;
        }
    }
    return std::nullopt;
}

bool CommandLineParser::validateRequiredOptions() const {
    std::vector<std::string> missing;
    for (const auto& opt : options) {
        if (opt.isRequired && !found(opt.longName)) {
            missing.push_back(opt.longName);
        }
    }

    if (!missing.empty()) {
        Ctrl::getInstance()->displayErrorMessage(910 + missing.size(), missing);
        return false;
    }
    return true;
}

// Validation functions
bool validateProbes(double r1, double r2, bool pm) {
    return !(pm && r2 < r1 && (Ctrl::getInstance()->displayErrorMessage(104), true));
}

bool validateExport(const std::string& out_dir, const std::vector<bool>& exp_options) {
    return !(isIncluded(true, exp_options) && out_dir.empty() && (Ctrl::getInstance()->displayErrorMessage(302), true));
}

bool validatePdb(const std::string& file, bool hetatm, bool unitcell) {
    return !((fileExtension(file) != "pdb" && fileExtension(file) != "cif") && (hetatm || unitcell) &&
            (Ctrl::getInstance()->displayErrorMessage(115), true));
}

// Define display options map
const std::map<std::string, unsigned> DISPLAY_OPTIONS = {
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
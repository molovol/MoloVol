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
            Ctrl::getInstance()->displayErrorMessage(901);
            return false;
        }

        // Process arguments directly without conversion to char*
        for (size_t i = 1; i < args.size(); i++) {
            const std::string& arg = args[i];
            if (arg.empty()) {
                Ctrl::getInstance()->displayErrorMessage(901);
                return false;
            }

            if (arg[0] != '-') continue;

            std::string optName = arg.substr(arg[1] == '-' ? 2 : 1);
            if (optName.empty()) {
                Ctrl::getInstance()->displayErrorMessage(901);
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
        Ctrl::getInstance()->displayErrorMessage(901);
        return false;
    }
}

bool CommandLineParser::parse(int argc, char* argv[]) {
    std::vector<std::string> args;
    for (int i = 0; i < argc; ++i) {
        args.push_back(argv[i]);
    }
    return parse(args);
}

bool CommandLineParser::found(const std::string& name) const {
    return parsedFlags.count(name) > 0 || parsedOptions.count(name) > 0;
}

std::optional<std::string> CommandLineParser::getValue(const std::string& name) const {
    auto it = parsedOptions.find(name);
    if (it != parsedOptions.end()) {
        return it->second;
    }
    return std::nullopt;
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

#ifndef EMSCRIPTEN
int main(int argc, char* argv[]) {
    auto ctrl = Ctrl::getInstance();
    ctrl->disableGUI(); // Ensure we're in CLI mode
    
    CommandLineParser parser;
    if (!parser.parse(argc, argv)) {
        return 1;
    }

    if (parser.found("help")) {
        parser.displayHelp();
        return 0;
    }

    if (parser.found("version")) {
        ctrl->version();
        return 0;
    }

    // Get required parameters with validation
    auto probe_radius = parser.getValue("radius");
    if (!probe_radius) {
        ctrl->displayErrorMessage(109);
        return 1;
    }
    double probe_radius_s = std::stod(*probe_radius);

    auto grid = parser.getValue("grid");
    if (!grid) {
        ctrl->displayErrorMessage(109);
        return 1;
    }
    double grid_resolution = std::stod(*grid);

    auto structure_file = parser.getValue("file-structure");
    if (!structure_file) {
        ctrl->displayErrorMessage(102);
        return 1;
    }

    // Optional parameters
    std::string elements_file_path = parser.getValue("file-elements").value_or(Ctrl::getDefaultElemPath());
    std::string output_dir_path = parser.getValue("dir-output").value_or("");
    
    double probe_radius_l = 0.0;
    if (auto radius2 = parser.getValue("radius2")) {
        probe_radius_l = std::stod(*radius2);
        if (probe_radius_l < probe_radius_s) {
            ctrl->displayErrorMessage(104);
            return 1;
        }
    }
    
    int tree_depth = 3; // Default value
    if (auto depth = parser.getValue("depth")) {
        tree_depth = std::stoi(*depth);
    }

    // Boolean flags
    bool opt_include_hetatm = parser.found("hetatm");
    bool opt_unit_cell = parser.found("unitcell");
    bool opt_surface_area = parser.found("surface");
    bool opt_probe_mode = parser.getValue("radius2").has_value();
    bool exp_report = parser.found("export-report");
    bool exp_total_map = parser.found("export-total");
    bool exp_cavity_maps = parser.found("export-cavities");

    // Set display options
    unsigned display_flag = mvOUT_ALL;
    if (auto output = parser.getValue("output")) {
        display_flag = evalDisplayOptions(*output);
    }

    // Handle quiet mode
    if (parser.found("quiet")) {
        ctrl->hush(true);
    }

    // Run the calculation
    bool success = ctrl->runCalculation(
        probe_radius_s,
        probe_radius_l,
        grid_resolution,
        *structure_file,
        elements_file_path,
        output_dir_path,
        tree_depth,
        opt_include_hetatm,
        opt_unit_cell,
        opt_surface_area,
        opt_probe_mode,
        exp_report,
        exp_total_map,
        exp_cavity_maps,
        display_flag
    );

    return success ? 0 : 1;
}
#endif
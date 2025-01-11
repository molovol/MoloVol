#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include "controller.h"
#include "flags.h"
#include "misc.h"
#include "base_cmdline.h"

using namespace emscripten;

// Initialize Controller for WASM environment
void init_controller() {
    Ctrl::getInstance()->disableGUI();
    Ctrl::getInstance()->hush(true);
}

// Get version information
std::string get_version() {
    init_controller();
    return Ctrl::getVersion();
}

// Main calculation function using CommandLineParser
val calculate_volumes(const std::vector<std::string>& args) {
    // Initialize controller
    init_controller();
    
    // Parse command line arguments using CommandLineParser
    CommandLineParser parser;
    // Debug print arguments
    std::string debug_args;
    for (const auto& arg : args) {
        debug_args += arg + " ";
    }
    printf("Parsing arguments: %s\n", debug_args.c_str());

    if (!parser.parse(args)) {
        val::global("Error").new_(std::string("Failed to parse arguments: " + debug_args)).throw_();
        return val::null();
    }

    // Check for required options
    if (!parser.found("radius") || !parser.found("grid") || !parser.found("file-structure")) {
        val::global("Error").new_(std::string("Missing required arguments")).throw_();
        return val::null();
    }

    // Get structure data from the file-structure argument
    auto structure_file = parser.getValue("file-structure").value();
    
    // Create temporary file for structure data
    std::string temp_filename = "/tmp/structure.tmp";
    FILE* fp = fopen(temp_filename.c_str(), "wb");
    if (!fp) {
        val::global("Error").new_(std::string("Failed to create temporary file")).throw_();
        return val::null();
    }
    fwrite(structure_file.c_str(), 1, structure_file.length(), fp);
    fclose(fp);

    // Extract values from parser
    double probe_radius_s = std::stod(parser.getValue("radius").value());
    double probe_radius_l = parser.found("radius2") ? std::stod(parser.getValue("radius2").value()) : 0.0;
    double grid_resolution = std::stod(parser.getValue("grid").value());
    int tree_depth = parser.found("depth") ? std::stoi(parser.getValue("depth").value()) : 4;
    
    // Get boolean flags
    bool include_hetatm = parser.found("hetatm");
    bool unit_cell = parser.found("unitcell");
    bool surface_area = parser.found("surface");
    bool export_report = parser.found("export-report");
    bool export_total_map = parser.found("export-total");
    bool export_cavity_maps = parser.found("export-cavities");

    // Get output flags
    unsigned output_flags = parser.found("output") ? 
        evalDisplayOptions(parser.getValue("output").value()) : 
        mvOUT_ALL;

    // Run calculation
    bool success = Ctrl::getInstance()->runCalculation(
        probe_radius_s,
        probe_radius_l,
        grid_resolution,
        temp_filename,
        Ctrl::getDefaultElemPath(),
        "/tmp",  // Temporary directory for any exports
        tree_depth,
        include_hetatm,
        unit_cell,
        surface_area,
        probe_radius_l > 0,  // probe mode
        export_report,
        export_total_map,
        export_cavity_maps,
        output_flags
    );

    if (!success) {
        val::global("Error").new_(std::string("Calculation failed")).throw_();
        return val::null();
    }

    // Check if calculation was completed
    if (!Ctrl::getInstance()->isCalculationDone()) {
        val::global("Error").new_(std::string("Calculation was aborted")).throw_();
        return val::null();
    }

    // Return success
    val result = val::object();
    result.set("success", true);
    result.set("version", Ctrl::getVersion());
    
    return result;
}

// Bind everything to JavaScript
EMSCRIPTEN_BINDINGS(molovol_module) {
    register_vector<std::string>("VectorString");
    
    value_object<CommandLineOption>("CommandLineOption")
        .field("shortName", &CommandLineOption::shortName)
        .field("longName", &CommandLineOption::longName)
        .field("description", &CommandLineOption::description)
        .field("isSwitch", &CommandLineOption::isSwitch)
        .field("isRequired", &CommandLineOption::isRequired)
        ;

    register_vector<CommandLineOption>("VectorCommandLineOption");

    class_<CommandLineParser>("CommandLineParser")
        .constructor<>()
        .function("parse", select_overload<bool(const std::vector<std::string>&)>(&CommandLineParser::parse))
        .function("found", &CommandLineParser::found)
        .function("getValue", &CommandLineParser::getValue)
        .function("displayHelp", &CommandLineParser::displayHelp)
        .function("getOptions", &CommandLineParser::getOptions)
        ;

    function("get_version", &get_version);
    function("calculate_volumes", &calculate_volumes);
}
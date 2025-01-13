#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <fstream>
#include "controller.h"
#include "flags.h"
#include "misc.h"
#include "base_cmdline.h"

using namespace emscripten;

// Structure to hold calculation parameters
struct CalculationParams {
    // Required parameters
    double probe_radius_small;
    double grid_resolution;
    std::string structure_content;
    std::string filename;
    // Optional parameters
    double probe_radius_large = 0.0;
    int tree_depth = 4;
    
    // Boolean flags
    bool include_hetatm = false;
    bool unit_cell = false;
    bool surface_area = false;
    bool export_report = false;
    bool export_total_map = false;
    bool export_cavity_maps = false;
};

// Initialize Controller for WASM environment
void init_controller() {
    Ctrl::getInstance()->disableGUI();
    Ctrl::getInstance()->hush(false);  // Enable output for debugging
}

// Get version information
std::string get_version() {
    init_controller();
    return Ctrl::getVersion();
}

// Helper function to write file content and verify it
bool writeAndVerifyFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file) {
        printf("Failed to open file for writing: %s\n", filename.c_str());
        return false;
    }
    
    file << content;
    file.close();
    
    // Verify file was written
    std::ifstream verify(filename);
    if (!verify) {
        printf("Failed to verify file existence: %s\n", filename.c_str());
        return false;
    }
    
    return true;
}

// Main calculation function using direct parameter passing
val calculate_volumes_direct(const CalculationParams& params) {
    printf("\n=== Starting calculation with direct parameter binding ===\n");
    
    // Initialize controller
    init_controller();
    
    // Validate parameters
    if (params.probe_radius_small <= 0) {
        val::global("Error").new_(std::string("Invalid probe radius")).throw_();
        return val::null();
    }
    
    if (params.grid_resolution <= 0 || params.grid_resolution < 0.1) {
        val::global("Error").new_(std::string("Invalid grid resolution")).throw_();
        return val::null();
    }
    
    if (params.structure_content.empty()) {
        val::global("Error").new_(std::string("No structure content provided")).throw_();
        return val::null();
    }
    

    
    // Set output flags for all relevant information
    unsigned output_flags = mvOUT_RESOLUTION | mvOUT_DEPTH | mvOUT_RADIUS_S | mvOUT_RADIUS_L | 
                          mvOUT_OPT | mvOUT_VOL | mvOUT_SURF | mvOUT_CAVITIES;
        
 // Write structure content to temporary file
    std::string input_filepath = "/tmp/" + params.filename;
    if (!writeAndVerifyFile(input_filepath, params.structure_content)) {
        val::global("Error").new_(std::string("Failed to write structure file")).throw_();
        return val::null();
    }
    // Run calculation
    bool success = false;
    try {
        success = Ctrl::getInstance()->runCalculation(
            params.probe_radius_small,
            params.probe_radius_large,
            params.grid_resolution,
            input_filepath,//strucutre input file
            "inputfile/elements.txt",//elements
            "/tmp",  // Temporary directory for any exports
            params.tree_depth,
            params.include_hetatm,
            params.unit_cell,
            params.surface_area,
            params.probe_radius_large > 0,  // probe mode
            params.export_report,
            params.export_total_map,
            params.export_cavity_maps,
            output_flags
        );
        
        printf("Calculation %s\n", success ? "succeeded" : "failed");
        
    } catch (const std::exception& e) {
        printf("Calculation failed with exception: %s\n", e.what());
        val::global("Error").new_(std::string("Calculation failed with exception: ") + e.what()).throw_();
        return val::null();
    } catch (...) {
        printf("Calculation failed with unknown exception\n");
        val::global("Error").new_(std::string("Calculation failed with unknown exception")).throw_();
        return val::null();
    }
    
    if (!success) {
        printf("Calculation failed - checking completion status\n");
        if (!Ctrl::getInstance()->isCalculationDone()) {
            printf("Calculation was not completed\n");
        }
        val::global("Error").new_(std::string("Calculation failed - Check input file format and parameters")).throw_();
        return val::null();
    }
    
    printf("Calculation completed successfully\n");
    
    // Return success
    val result = val::object();
    result.set("success", true);
    result.set("version", Ctrl::getVersion());
    
    return result;
}

// Bind everything to JavaScript
EMSCRIPTEN_BINDINGS(molovol_module) {
    value_object<CalculationParams>("CalculationParams")
        .field("probe_radius_small", &CalculationParams::probe_radius_small)
        .field("grid_resolution", &CalculationParams::grid_resolution)
        .field("structure_content", &CalculationParams::structure_content)
        .field("filename", &CalculationParams::filename)
        .field("probe_radius_large", &CalculationParams::probe_radius_large)
        .field("tree_depth", &CalculationParams::tree_depth)
        .field("include_hetatm", &CalculationParams::include_hetatm)
        .field("unit_cell", &CalculationParams::unit_cell)
        .field("surface_area", &CalculationParams::surface_area)
        .field("export_report", &CalculationParams::export_report)
        .field("export_total_map", &CalculationParams::export_total_map)
        .field("export_cavity_maps", &CalculationParams::export_cavity_maps);

    function("get_version", &get_version);
    function("calculate_volumes", &calculate_volumes_direct);
}
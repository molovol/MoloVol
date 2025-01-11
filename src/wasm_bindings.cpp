#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include "controller.h"
#include "flags.h"
#include "misc.h"

using namespace emscripten;

// Input options structure for JavaScript
struct MoloVolOptions {
    double probe_radius_s;       // Required: small probe radius
    double grid_resolution;      // Required: grid resolution
    double probe_radius_l;       // Optional: large probe radius (for two-probe mode)
    int tree_depth;             // Optional: octree depth (default: 4)
    bool include_hetatm;        // Optional: include HETATM from PDB
    bool unit_cell;             // Optional: analyze unit cell
    bool surface_area;          // Optional: calculate surfaces
    bool export_report;         // Optional: export report
    bool export_total_map;      // Optional: export cavity maps
    bool export_cavity_maps;    // Optional: export cavity maps
    unsigned output_flags;      // Output control flags
};

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

// Main calculation function
val calculate_volumes(std::string structure_data, const MoloVolOptions& options) {
    // Initialize controller
    init_controller();
    
    // Create temporary file for structure data
    std::string temp_filename = "/tmp/structure.tmp";
    FILE* fp = fopen(temp_filename.c_str(), "wb");
    if (!fp) {
        val::global("Error").new_(std::string("Failed to create temporary file")).throw_();
        return val::null();
    }
    fwrite(structure_data.c_str(), 1, structure_data.length(), fp);
    fclose(fp);

    // Run calculation
    bool success = Ctrl::getInstance()->runCalculation(
        options.probe_radius_s,
        options.probe_radius_l,
        options.grid_resolution,
        temp_filename,
        Ctrl::getDefaultElemPath(),
        "/tmp",  // Temporary directory for any exports
        options.tree_depth,
        options.include_hetatm,
        options.unit_cell,
        options.surface_area,
        options.probe_radius_l > 0,  // probe mode
        options.export_report,
        options.export_total_map,
        options.export_cavity_maps,
        options.output_flags
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
    
    // Note: Additional result data would need to be exposed through the Controller class
    // For now, we return success status and version
    
    return result;
}

// Convert output format string to flags
unsigned parse_output_format(const std::string& format) {
    if (format.empty() || format == "all") {
        return mvOUT_ALL;
    }
    
    unsigned flags = mvOUT_NONE;
    std::istringstream ss(format);
    std::string token;
    
    while (std::getline(ss, token, ',')) {
        if (token == "inputfile") flags |= mvOUT_STRUCTURE;
        else if (token == "resolution") flags |= mvOUT_RESOLUTION;
        else if (token == "depth") flags |= mvOUT_DEPTH;
        else if (token == "radius_small") flags |= mvOUT_RADIUS_S;
        else if (token == "radius_large") flags |= mvOUT_RADIUS_L;
        else if (token == "vol") flags |= mvOUT_VOL;
        else if (token == "surf") flags |= mvOUT_SURF;
        else if (token == "cavities") flags |= mvOUT_CAVITIES;
    }
    
    return flags;
}

// Bind everything to JavaScript
EMSCRIPTEN_BINDINGS(molovol_module) {
    value_object<MoloVolOptions>("MoloVolOptions")
        .field("probe_radius_s", &MoloVolOptions::probe_radius_s)
        .field("grid_resolution", &MoloVolOptions::grid_resolution)
        .field("probe_radius_l", &MoloVolOptions::probe_radius_l)
        .field("tree_depth", &MoloVolOptions::tree_depth)
        .field("include_hetatm", &MoloVolOptions::include_hetatm)
        .field("unit_cell", &MoloVolOptions::unit_cell)
        .field("surface_area", &MoloVolOptions::surface_area)
        .field("export_report", &MoloVolOptions::export_report)
        .field("export_total_map", &MoloVolOptions::export_total_map)
        .field("export_cavity_maps", &MoloVolOptions::export_cavity_maps)
        .field("output_flags", &MoloVolOptions::output_flags)
        ;
    
    function("get_version", &get_version);
    function("calculate_volumes", &calculate_volumes);
    function("parse_output_format", &parse_output_format);
}
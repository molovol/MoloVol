#include "controller.h"
#include "model.h"
#include "misc.h"
#include <string>

// Initialize static members
Ctrl* Ctrl::s_instance = nullptr;

// Get singleton instance
Ctrl* Ctrl::getInstance() {
    if (s_instance == nullptr) {
        s_instance = new Ctrl();
    }
    return s_instance;
}

// Version info
std::string Ctrl::getVersion() {
    return s_version;
}

// Default elements file path
std::string Ctrl::getDefaultElemPath() {
    return getResourcesDir() + "/" + s_elem_file;
}

// GUI control functions
void Ctrl::disableGUI() {
    _to_gui = false;
}

void Ctrl::hush(const bool quiet) {
    _quiet = quiet;
}

// Main calculation function for WASM
bool Ctrl::runCalculation(
    const double probe_radius_s,
    const double probe_radius_l,
    const double grid_resolution,
    const std::string& structure_file_path,
    const std::string& elements_file_path,
    const std::string& output_dir_path,
    const int tree_depth,
    const bool opt_include_hetatm,
    const bool opt_unit_cell,
    const bool opt_surface_area,
    const bool opt_probe_mode,
    const bool exp_report,
    const bool exp_total_map,
    const bool exp_cavity_maps,
    const unsigned display_flag) {

    if (_current_calculation == nullptr) {
        _current_calculation = new Model();
    }

    // Initialize calculation
    _calculation_finished = false;
    
    try {
        if (!_current_calculation->readAtomsFromFile(structure_file_path, opt_include_hetatm)) {
            return false;
        }
    } catch (const std::exception& e) {
        return false;
    }

    std::vector<std::string> included_elements = _current_calculation->listElementsInStructure();

    if (!_current_calculation->importElemFile(elements_file_path)) {
        return false;
    }

    if (!_current_calculation->setParameters(
            structure_file_path,
            output_dir_path,
            opt_include_hetatm,
            opt_unit_cell,
            opt_surface_area,
            opt_probe_mode,
            probe_radius_s,
            probe_radius_l,
            grid_resolution,
            tree_depth,
            exp_report,
            exp_total_map,
            exp_cavity_maps,
            _current_calculation->getRadiusMap(),
            included_elements)) {
        return false;
    }

    // Run calculation
    CalcReportBundle data = _current_calculation->generateData();
    _calculation_finished = data.success;

    return data.success;
}

// Calculation status
bool Ctrl::isCalculationDone() {
    return _calculation_finished;
}
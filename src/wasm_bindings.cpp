# include <emscripten/bind.h>
# include <emscripten/val.h>
# include "molovol.h"
# include <vector>
# include <string>
# include <memory>

using namespace emscripten;

// Structure to hold calculation options
struct CalculationOptions {
    bool hetatm;
    bool unitcell;
    bool surface;
    bool exportReport;
    bool exportTotal;
    bool exportCavities;
};

// Structure to hold calculation results
struct CalculationResult {
    double resolution;
    int depth;
    double radius_small;
    double radius_large;
    struct {
        double total;
        double accessible;
        double contact;
    } volume;
    struct {
        double total;
        double accessible;
        double contact;
    } surface;
    std::vector<struct {
        std::string id;
        double volume;
        double surface;
    }> cavities;
};

// Main calculation function exposed to JavaScript
CalculationResult calculate_volumes(val structureData, 
                                  double radius, 
                                  double grid, 
                                  val radius2Option,
                                  const CalculationOptions& options) {
    // Convert TypedArray to vector
    std::vector<uint8_t> fileData;
    const size_t length = structureData["length"].as<size_t>();
    fileData.resize(length);
    val memory = val::global("Uint8Array").new_(val::global("WebAssembly")["Memory"]["buffer"]);
    memory.call<void>("set", structureData);
    
    // Create temporary file to work with existing MoloVol code
    std::string tempFilename = "/tmp/structure.xyz";  // In-memory filesystem
    FILE* fp = fopen(tempFilename.c_str(), "wb");
    fwrite(fileData.data(), 1, fileData.size(), fp);
    fclose(fp);

    // Initialize MoloVol with parameters
    MoloVol calculator;
    calculator.setProbeRadius(radius);
    calculator.setGridSpacing(grid);
    
    if (!radius2Option.isUndefined()) {
        calculator.setLargeProbeRadius(radius2Option.as<double>());
    }

    // Set options
    calculator.setIncludeHETATM(options.hetatm);
    calculator.setAnalyzeUnitCell(options.unitcell);
    calculator.setCalculateSurface(options.surface);
    
    // Run calculation
    try {
        calculator.loadStructure(tempFilename);
        calculator.calculate();
        
        // Prepare results
        CalculationResult result;
        result.resolution = calculator.getResolution();
        result.depth = calculator.getDepth();
        result.radius_small = radius;
        result.radius_large = radius2Option.isUndefined() ? 0.0 : radius2Option.as<double>();
        
        // Get volumes
        auto volumes = calculator.getVolumes();
        result.volume.total = volumes.total;
        result.volume.accessible = volumes.accessible;
        result.volume.contact = volumes.contact;
        
        // Get surface areas if calculated
        if (options.surface) {
            auto surfaces = calculator.getSurfaces();
            result.surface.total = surfaces.total;
            result.surface.accessible = surfaces.accessible;
            result.surface.contact = surfaces.contact;
        }
        
        // Get cavity information
        auto cavities = calculator.getCavities();
        for (const auto& cavity : cavities) {
            result.cavities.push_back({
                cavity.getId(),
                cavity.getVolume(),
                options.surface ? cavity.getSurfaceArea() : 0.0
            });
        }
        
        return result;
    }
    catch (const std::exception& e) {
        // Convert C++ exceptions to JavaScript errors
        val::global("Error").new_(std::string("Calculation failed: ") + e.what()).throw_();
        return CalculationResult{};  // Never reached, but satisfies compiler
    }
}

// Bind structures and functions to JavaScript
EMSCRIPTEN_BINDINGS(molovol_module) {
    value_object<CalculationOptions>("CalculationOptions")
        .field("hetatm", &CalculationOptions::hetatm)
        .field("unitcell", &CalculationOptions::unitcell)
        .field("surface", &CalculationOptions::surface)
        .field("exportReport", &CalculationOptions::exportReport)
        .field("exportTotal", &CalculationOptions::exportTotal)
        .field("exportCavities", &CalculationOptions::exportCavities)
        ;
        
    value_object<CalculationResult>("CalculationResult")
        .field("resolution", &CalculationResult::resolution)
        .field("depth", &CalculationResult::depth)
        .field("radius_small", &CalculationResult::radius_small)
        .field("radius_large", &CalculationResult::radius_large)
        .field("volume", &CalculationResult::volume)
        .field("surface", &CalculationResult::surface)
        .field("cavities", &CalculationResult::cavities)
        ;
        
    function("calculate_volumes", &calculate_volumes);
}

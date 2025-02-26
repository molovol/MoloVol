// Import WASM module - using importScripts for web worker compatibility
importScripts('molovol_wasm.js');

// Configure module
const moduleConfig = {
    print: (text) => {
        self.postMessage({ type: 'output', data: text });
    },
    printErr: (text) => {
        self.postMessage({ type: 'error', data: text });
    }
};

// Initialize WASM module, name is defined in wasm compilation config in cmake
let wasmModule;
createMoloVolModule(moduleConfig).then(module => {
    wasmModule = module;
    
    // Get and send version to main thread
    try {
        const version = wasmModule.get_version();
        self.postMessage({ 
            type: 'version', 
            data: version 
        });
    } catch (error) {
        console.error('Failed to get version:', error);
    }
    
    self.postMessage({ type: 'ready' });
}).catch(error => {
    self.postMessage({ type: 'error', data: `Failed to initialize WASM: ${error}` });
});

// Handle messages from main thread
self.onmessage = async function(e) {
    if (e.data.type !== 'calculate' || !wasmModule) return;

    try {
        // Run calculation
        const result = wasmModule.calculate_volumes(e.data.params);
        
        // Notify main thread of completion
        self.postMessage({ 
            type: 'result',
            data: result
        });
    } catch (error) {
        self.postMessage({ 
            type: 'error', 
            data: error.toString()
        });
    }
};
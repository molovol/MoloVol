// molovol.js
import init, { calculate_volumes } from './molovol_wasm.js';

// Constants for file size and cleanup
const MAX_STORAGE_SIZE = 4 * 1024 * 1024 * 1024; // 4 GB
const TARGET_STORAGE_SIZE = 3 * 1024 * 1024 * 1024; // 3 GB
const GRACE_PERIOD_MS = 600000; // 10 minutes in milliseconds

// Initialize WebAssembly module
let wasmModule;
init().then(module => {
    wasmModule = module;
}).catch(err => {
    console.error("Failed to initialize WASM module:", err);
});

// File type validation
const ALLOWED_EXTENSIONS = new Set(['xyz', 'pdb', 'cif']);
function validateExtension(filename) {
    const ext = filename.split('.').pop().toLowerCase();
    return ALLOWED_EXTENSIONS.has(ext);
}

// Error handling
const ERR_PREFIX = "E.";
const ErrorMessages = {
    NOFILE: `${ERR_PREFIX}No file was uploaded and previous file could not be used`,
    NORADIUS: `${ERR_PREFIX}Small probe radius must be specified`,
    NOGRID: `${ERR_PREFIX}Spatial resolution must be specified`,
    SMALLGRID: `${ERR_PREFIX}If you would like to use a spatial resolution of less than 0.1, please use the desktop application`,
    FORMAT: `${ERR_PREFIX}File format of uploaded file is not supported`
};

// File storage management using IndexedDB
class StorageManager {
    constructor() {
        this.dbName = 'MoloVolStorage';
        this.storeName = 'files';
        this.db = null;
    }

    async init() {
        return new Promise((resolve, reject) => {
            const request = indexedDB.open(this.dbName, 1);
            
            request.onerror = () => reject(request.error);
            request.onsuccess = () => {
                this.db = request.result;
                resolve();
            };
            
            request.onupgradeneeded = (event) => {
                const db = event.target.result;
                if (!db.objectStoreNames.contains(this.storeName)) {
                    db.createObjectStore(this.storeName, { keyPath: 'id' });
                }
            };
        });
    }

    async storeFile(file, metadata = {}) {
        const id = crypto.randomUUID();
        const timestamp = Date.now();
        
        return new Promise((resolve, reject) => {
            const transaction = this.db.transaction([this.storeName], 'readwrite');
            const store = transaction.objectStore(this.storeName);
            
            const reader = new FileReader();
            reader.onload = () => {
                const fileData = {
                    id,
                    name: file.name,
                    type: file.type,
                    size: file.size,
                    data: reader.result,
                    timestamp,
                    metadata
                };
                
                store.add(fileData).onsuccess = () => resolve(id);
            };
            reader.onerror = () => reject(reader.error);
            reader.readAsArrayBuffer(file);
        });
    }

    async cleanup() {
        const currentSize = await this.getTotalSize();
        if (currentSize <= TARGET_STORAGE_SIZE) return;

        const transaction = this.db.transaction([this.storeName], 'readwrite');
        const store = transaction.objectStore(this.storeName);
        const files = await this.getAllFiles();

        // Sort by timestamp
        files.sort((a, b) => a.timestamp - b.timestamp);

        let deletedSize = 0;
        const cutoffTime = Date.now() - GRACE_PERIOD_MS;

        for (const file of files) {
            if (currentSize - deletedSize <= TARGET_STORAGE_SIZE &&
                file.timestamp > cutoffTime) {
                break;
            }
            
            await store.delete(file.id);
            deletedSize += file.size;
        }
    }

    async getTotalSize() {
        const files = await this.getAllFiles();
        return files.reduce((total, file) => total + file.size, 0);
    }

    async getAllFiles() {
        return new Promise((resolve, reject) => {
            const transaction = this.db.transaction([this.storeName], 'readonly');
            const store = transaction.objectStore(this.storeName);
            const request = store.getAll();
            
            request.onsuccess = () => resolve(request.result);
            request.onerror = () => reject(request.error);
        });
    }
}

// Main MoloVol handler class
export class MoloVolHandler {
    constructor() {
        this.storage = new StorageManager();
        this.initPromise = this.storage.init();
    }

    async processStructure(file, options) {
        await this.initPromise;
        
        // Validate file
        if (!validateExtension(file.name)) {
            throw new Error(ErrorMessages.FORMAT);
        }

        // Validate required options
        if (!options.radius) {
            throw new Error(ErrorMessages.NORADIUS);
        }
        if (!options.grid) {
            throw new Error(ErrorMessages.NOGRID);
        }
        if (parseFloat(options.grid) < 0.1) {
            throw new Error(ErrorMessages.SMALLGRID);
        }

        // Store file
        await this.storage.cleanup();
        const fileId = await this.storage.storeFile(file);

        // Prepare calculation parameters
        const params = {
            fileId,
            radius: parseFloat(options.radius),
            grid: parseFloat(options.grid),
            radius2: options.radius2 ? parseFloat(options.radius2) : null,
            hetatm: options.hetatm || false,
            unitcell: options.unitcell || false,
            surface: options.surface || false,
            exportReport: options.exportReport || false,
            exportTotal: options.exportTotal || false,
            exportCavities: options.exportCavities || false
        };

        // Execute calculation
        try {
            const result = await this.executeCalculation(params);
            return this.formatResults(result, file.name);
        } catch (error) {
            throw new Error(`${ERR_PREFIX}${error.message}`);
        }
    }

    async executeCalculation(params) {
        // Get file data from storage
        const fileData = await this.getFileData(params.fileId);
        
        // Convert parameters to WASM-compatible format
        const wasmParams = {
            structure: new Uint8Array(fileData),
            radius: params.radius,
            grid: params.grid,
            radius2: params.radius2,
            options: {
                hetatm: params.hetatm,
                unitcell: params.unitcell,
                surface: params.surface,
                exportReport: params.exportReport,
                exportTotal: params.exportTotal,
                exportCavities: params.exportCavities
            }
        };

        // Execute WASM calculation
        return calculate_volumes(wasmParams);
    }

    async getFileData(fileId) {
        const transaction = this.storage.db.transaction([this.storage.storeName], 'readonly');
        const store = transaction.objectStore(this.storage.storeName);
        
        return new Promise((resolve, reject) => {
            const request = store.get(fileId);
            request.onsuccess = () => resolve(request.result.data);
            request.onerror = () => reject(request.error);
        });
    }

    formatResults(result, filename) {
        let output = `Results for structure file: ${filename}\n`;
        
        // Add calculation parameters
        output += `Resolution: ${result.resolution} Å\n`;
        output += `Depth: ${result.depth}\n`;
        output += `Small probe radius: ${result.radius_small} Å\n`;
        if (result.radius_large) {
            output += `Large probe radius: ${result.radius_large} Å\n`;
        }
        
        // Add volumes and surface areas
        output += `\n<VOLUMES>\n`;
        output += `Total volume: ${result.volume.total.toFixed(1)} Å³\n`;
        if (result.surface) {
            output += `Surface area: ${result.surface.total.toFixed(1)} Å²\n`;
        }
        
        // Add cavity information if present
        if (result.cavities && result.cavities.length > 0) {
            output += `\nCavity ID  Volume      `;
            if (result.surface) {
                output += `Surface     `;
            }
            output += `\n           (Å³)        `;
            if (result.surface) {
                output += `(Å²)        `;
            }
            output += `\n`;
            
            result.cavities.forEach(cavity => {
                output += `${cavity.id.padEnd(10)} ${cavity.volume.toFixed(1).padEnd(12)}`;
                if (result.surface) {
                    output += `${cavity.surface.toFixed(1).padEnd(12)}`;
                }
                output += `\n`;
            });
        }

        return output;
    }
}

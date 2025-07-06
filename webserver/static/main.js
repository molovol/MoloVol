// Initialize output and worker
let currentActivity = '';
let currentProgress = 0;
let animationFrameId = null;
let pendingCalcUpdates = [];

// Create and initialize web worker
const worker = new Worker('static/worker.js');
worker.onmessage = function(e) {
    const { type, data } = e.data;
    switch (type) {
        case 'ready':
            console.log('WASM worker ready');
            break;
        case 'version':
            // Update version display
            const versionElement = document.getElementById('version');
            if (versionElement) {
                versionElement.textContent = 'v' + data;
            }
            break;
        case 'output':
            // More precise detection of progress messages
            const isProgress = 
                /^\d+%$/.test(data) || 
                data.includes('Searching inaccessible areas') ||
                data.includes('Probing space') ||
                data.includes('Blocking off cavities') ||
                data.includes('Identifying cavities');
                
            appendOutput(data, isProgress);
            break;
        case 'error':
            appendOutput(`Error: ${data}`, true);
            break;
        case 'result':
            handleCalculationComplete(data);
            break;
    }
};

function updateProgressBar(activity, progress) {
    const progressOutput = document.getElementById('progress-output');
    if (!progressOutput) return;

    // Create or update progress elements
    if (!progressOutput.querySelector('.progress-activity')) {
        progressOutput.innerHTML = `
            <span class="progress-activity"></span>
            <span class="progress-percentage"></span>
        `;
    }

    // Update progress content
    const activityLabel = progressOutput.querySelector('.progress-activity');
    const percentageLabel = progressOutput.querySelector('.progress-percentage');

    if (activity) {
        currentActivity = activity;
        currentProgress = 0;
        activityLabel.textContent = activity;
        percentageLabel.textContent = '0%';
    }

    if (progress !== undefined) {
        currentProgress = progress;
        percentageLabel.textContent = `${progress}%`;
    }
}

function appendOutput(text, isProgress = false) {
    if (isProgress) {
        // Check if text contains an activity name
        const activityMatch = text.match(/(Probing space|Identifying cavities|Searching inaccessible areas|Blocking off cavities)\.{3}/);
        if (activityMatch) {
            updateProgressBar(activityMatch[1], 0);
        } else {
            // Check if text contains a percentage
            const percentMatch = text.match(/(\d+)%/);
            if (percentMatch) {
                updateProgressBar(null, parseInt(percentMatch[1]));
            }
        }
    } else {
        const calcResultElement = document.getElementById('calculation-result');
        if (!calcResultElement) return;

        // Check if we already have the table container
        let tableContainer = calcResultElement.querySelector('.cavities-container');
        if (!tableContainer) {
            // Create the containers for both text and table
            const textDiv = document.createElement('div');
            textDiv.className = 'calculation-text';
            tableContainer = document.createElement('div');
            tableContainer.className = 'cavities-container';
            
            // Create table structure
            const table = document.createElement('table');
            table.className = 'cavities';
            table.innerHTML = `
                <thead>
                    <tr id="table-header"></tr>
                </thead>
                <tbody id="table-body"></tbody>
            `;
            
            tableContainer.appendChild(table);
            calcResultElement.appendChild(textDiv);
            calcResultElement.appendChild(tableContainer);
        }

        if (detectTableContent(text)) {
            const table = tableContainer.querySelector('table.cavities');
            
            if (text.includes('Cavity ID')) {
                // Handle header row
                const headers = ['Cavity ID', 'Occupied Volume (A^3)', 'Cavity Type', 'Center Coord x, y, z (A)'];
                const headerRow = table.querySelector('#table-header');
                if (headerRow) {
                    headerRow.innerHTML = headers.map(h => `<th>${h}</th>`).join('');
                    table.style.display = 'table';
                }
            } else {
                // Handle data row
                const tbody = table.querySelector('#table-body');
                if (tbody) {
                    // Split by whitespace but preserve content within parentheses
                    const parts = text.trim().split(/\s+/);
                    const cells = [];
                    let i = 0;
                    
                    // First three columns (ID, Volume, Type)
                    while (i < parts.length - 3) { // -3 for the coordinates
                        cells.push(parts[i]);
                        i++;
                    }
                    
                    // Last column (coordinates)
                    cells.push(parts.slice(i).join(' '));
                    
                    const tr = document.createElement('tr');
                    tr.innerHTML = cells.map(c => `<td>${c}</td>`).join('');
                    tbody.appendChild(tr);
                }
            }
        } else {
            // Handle non-table text
            const textDiv = calcResultElement.querySelector('.calculation-text');
            if (textDiv) {
                const div = document.createElement('div');
                div.textContent = text;
                textDiv.appendChild(div);
            }
        }
    }
}

function detectTableContent(text) {
    return text.includes('Cavity ID') || 
           (text.match(/^\d+\s+\d+\.\d+\s+/) && text.includes('Isolated'));
}

function handleCalculationComplete(result) {
    // Hide progress bar when calculation is complete
    const progressOutput = document.getElementById('progress-output');
    if (progressOutput) {
        progressOutput.style.display = 'none';
    }
    
    setupDownloadButtons(result);
    window.location.hash = 'output';
}

// Form submission handler
async function handleSubmit(event) {
    event.preventDefault();

    const fileInput = document.getElementById('structure');
    const file = fileInput.files[0];
    if (!file) {
        alert('Please select a structure file.');
        return;
    }

    try {
        // Clear previous output and show results section
        const outputElement = document.getElementById('calculation-result');
        outputElement.textContent = '';
        
        // Show and reset progress output
        const progressOutput = document.getElementById('progress-output');
        progressOutput.style.display = 'block';
        progressOutput.innerHTML = '';
        
        document.getElementById('results').style.display = 'block';
    
        // Read file content
        const fileContent = await file.text();
    
        // Create calculation parameters
        const params = {
            probe_radius_small: parseFloat(document.getElementById('radius').value),
            probe_radius_large: document.getElementById('radius2').value ? 
                               parseFloat(document.getElementById('radius2').value) : 0.0,
            grid_resolution: parseFloat(document.getElementById('grid').value),
            tree_depth: 4,
            structure_content: fileContent,
            filename: file.name,
            include_hetatm: document.getElementById('hetatm').checked,
            unit_cell: document.getElementById('unitcell').checked,
            surface_area: document.getElementById('surface').checked,
            export_report: true,
            export_total_map: document.getElementById('export-total').checked,
            export_cavity_maps: document.getElementById('export-cavities').checked
        };

        // Send calculation request to worker
        worker.postMessage({ type: 'calculate', params });
    
    } catch (error) {
        console.error('Submission failed:', error);
        appendOutput(`Error: ${error.message}`);
    }
}

// Initialize event listeners when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    const form = document.getElementById('inputs');
    form.addEventListener('submit', handleSubmit);
    initializeUI();
});
function initializeUI() {
    initializeFileInput();
    initializeProbeValidation();
    initializeDragAndDrop();
}

function initializeFileInput() {
    document.getElementById('structure').addEventListener('change', function(e) {
        evalFilename(this.value, true);
    });
}

function initializeProbeValidation() {
    const radius = document.getElementById("radius");
    const radius2 = document.getElementById("radius2");
    radius2.addEventListener("input", (event) => {
        if (radius2.value !== "" && parseFloat(radius2.value) < parseFloat(radius.value)) {
            radius2.setCustomValidity("Must be larger than small probe radius");
            radius2.reportValidity();
        } else {
            radius2.setCustomValidity("");
        }
    });
}

function initializeDragAndDrop() {
    const dropzone = document.getElementById('dropzone');
    const input = document.getElementById('structure');
    const fileNameDisplay = document.getElementById('file-name');

    function handleFileSelect(file) {
        if (file) {
            fileNameDisplay.textContent = file.name;
            dropzone.classList.add('has-file');
            evalFilename(file.name, true);
        } else {
            fileNameDisplay.textContent = 'No file selected';
            dropzone.classList.remove('has-file');
        }
    }

    input.addEventListener('change', function(e) {
        handleFileSelect(this.files[0]);
    });

    // Setup drag and drop handlers
    dropzone.addEventListener('click', function(e) {
        if (e.target !== input) {
            e.preventDefault();
            input.click();
        }
    });

    ['dragenter', 'dragover', 'dragleave', 'drop'].forEach(eventName => {
        dropzone.addEventListener(eventName, e => {
            e.preventDefault();
            e.stopPropagation();
        });
    });

    ['dragenter', 'dragover'].forEach(eventName => {
        dropzone.addEventListener(eventName, () => {
            dropzone.classList.add('drag-over');
        });
    });

    ['dragleave', 'drop'].forEach(eventName => {
        dropzone.addEventListener(eventName, () => {
            dropzone.classList.remove('drag-over');
        });
    });

    dropzone.addEventListener('drop', function(e) {
        const file = e.dataTransfer.files[0];
        if (file) {
            input.files = e.dataTransfer.files;
            handleFileSelect(file);
        }
    });
}

function evalFilename(filename, newFile) {
    const extension = filename.match(/\.\w+$/gm);
    if (!extension) {
        lockCheckboxes(false, false);
        alert('File extension unknown.');
        return;
    }

    switch (extension[0].toLowerCase()) {
        case '.pdb':
            if (newFile) {
                document.getElementById('hetatm').checked = true;
            }
            lockCheckboxes(false, false);
            break;
        case '.xyz':
            lockCheckboxes(true, true);
            break;
        case '.cif':
            lockCheckboxes(true, false);
            break;
        default:
            lockCheckboxes(false, false);
            alert('File extension unknown.');
    }
}

function lockCheckboxes(htDisable, ucDisable) {
    document.getElementById('hetatm').disabled = htDisable;
    document.getElementById('unitcell').disabled = ucDisable;
}

function handleCalculationComplete(result) {
    setupDownloadButtons(result);
    window.location.hash = 'output';
}

function setupDownloadButtons(result) {
    // Implementation of download button setup...
    // This will depend on how your WASM module returns results
}

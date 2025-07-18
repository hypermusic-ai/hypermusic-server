let rootFeatureName;
let executeResultData = []; // populated during execute()

// --------------------------------------------------------------------------
// MIDI
// --------------------------------------------------------------------------
class MIDIEvent {
    constructor(tick) {
        this.tick = tick;
    }
}

class MIDINoteEvent extends MIDIEvent {
    constructor(tick, pitch, velocity, duration) {
        super(tick);
        this.pitch = pitch;
        this.velocity = velocity;
        this.duration = duration;
    }
}

class MIDITimeSignatureEvent extends MIDIEvent {
    constructor(tick, numerator, denominator) {
        super(tick);
        this.numerator = numerator;
        this.denominator = denominator;
    }
}

class MIDIBPMEvent extends MIDIEvent {
    constructor(tick, bpm) {
        super(tick);
        this.bpm = bpm;
    }
}

let availableFeatureNames = []; // Global store for dropdown values
let featureDropdownElements = [];

function populateFeatureDropdown(select) {
    const currentValue = select.value;

    // Clear old options
    select.innerHTML = '';

    const emptyOption = document.createElement('option');
    emptyOption.value = '';
    emptyOption.textContent = '— select feature —';
    select.appendChild(emptyOption);

    availableFeatureNames.forEach(fname => {
        const option = document.createElement('option');
        option.value = fname;
        option.textContent = fname;
        select.appendChild(option);
    });

    // Try to restore previous selection if still valid
    if (availableFeatureNames.includes(currentValue)) {
        select.value = currentValue;
    }
}

function createFeatureDropdown(name) {
    const wrapper = document.createElement('div');
    wrapper.style.display = 'flex';
    wrapper.style.alignItems = 'center';
    wrapper.style.gap = '0.5rem';
    wrapper.style.marginBottom = '0.5rem';

    const label = document.createElement('label');
    label.textContent = name;
    label.style.minWidth = '80px';
    label.style.color = '#ccc';
    label.style.margin = 0;
    label.style.width = 'auto';

    const select = document.createElement('select');
    select.name = name;
    select.style.flex = '1';
    select.style.background = '#1e1e1e';
    select.style.color = '#fff';
    select.style.border = '1px solid #444';
    select.style.borderRadius = '4px';
    select.style.padding = '0.4rem';

    wrapper.appendChild(label);
    wrapper.appendChild(select);

    // Save reference for later updates
    featureDropdownElements.push(select);

    // Populate initially
    populateFeatureDropdown(select);

    return wrapper;
}

function createEventButton(label, onClick) {
    const btn = document.createElement('button');
    btn.textContent = label;
    btn.style.marginRight = '0.5rem';
    btn.style.marginBottom = '0.5rem';
    btn.onclick = onClick;
    return btn;
}

function renderMIDINoteEvent(container) {
    const div = document.createElement('div');
    div.className = 'midi-event note';
    div.style.border = '1px solid #333';
    div.style.padding = '0.5rem';
    div.style.marginBottom = '0.5rem';
    div.style.borderRadius = '4px';
    div.style.backgroundColor = '#222';

    const titleRow = document.createElement('div');
    titleRow.style.display = 'flex';
    titleRow.style.justifyContent = 'space-between';
    titleRow.style.alignItems = 'center';
    titleRow.style.marginBottom = '0.5rem';

    const title = document.createElement('strong');
    title.textContent = '🎵 Note Event';
    title.style.color = '#fff';

    const removeBtn = document.createElement('button');
    removeBtn.textContent = '✖';
    removeBtn.style.background = '#444';
    removeBtn.style.color = '#fff';
    removeBtn.style.border = 'none';
    removeBtn.style.borderRadius = '4px';
    removeBtn.style.padding = '0.3rem 0.6rem';
    removeBtn.style.cursor = 'pointer';
    removeBtn.onclick = () => container.removeChild(div);

    titleRow.appendChild(title);
    titleRow.appendChild(removeBtn);
    div.appendChild(titleRow);

    const separator = document.createElement('hr');
    separator.style.border = '0';
    separator.style.borderTop = '1px solid #444';
    separator.style.margin = '0.5rem 0';
    div.appendChild(separator);

    div.appendChild(createFeatureDropdown('tick'));
    div.appendChild(createFeatureDropdown('pitch'));
    div.appendChild(createFeatureDropdown('velocity'));
    div.appendChild(createFeatureDropdown('duration'));

    container.appendChild(div);
}

function renderMIDITimeSignatureEvent(container) {
    const div = document.createElement('div');
    div.className = 'midi-event timesig';
    div.style.border = '1px solid #333';
    div.style.padding = '0.5rem';
    div.style.marginBottom = '0.5rem';
    div.style.borderRadius = '4px';
    div.style.backgroundColor = '#223344';

    const titleRow = document.createElement('div');
    titleRow.style.display = 'flex';
    titleRow.style.justifyContent = 'space-between';
    titleRow.style.alignItems = 'center';
    titleRow.style.marginBottom = '0.5rem';

    const title = document.createElement('strong');
    title.textContent = '🕒 Time Signature Event';
    title.style.fontSize = '1rem';
    title.style.color = '#fff';

    const removeBtn = document.createElement('button');
    removeBtn.textContent = '✖';
    removeBtn.style.background = '#444';
    removeBtn.style.color = '#fff';
    removeBtn.style.border = 'none';
    removeBtn.style.borderRadius = '4px';
    removeBtn.style.padding = '0.3rem 0.6rem';
    removeBtn.style.cursor = 'pointer';
    removeBtn.onclick = () => container.removeChild(div);

    titleRow.appendChild(title);
    titleRow.appendChild(removeBtn);
    div.appendChild(titleRow);

    const separator = document.createElement('hr');
    separator.style.border = '0';
    separator.style.borderTop = '1px solid #444';
    separator.style.margin = '0.5rem 0';
    div.appendChild(separator);

    div.appendChild(createFeatureDropdown('tick'));
    div.appendChild(createFeatureDropdown('numerator'));
    div.appendChild(createFeatureDropdown('denominator'));

    container.appendChild(div);
}

function renderMIDIBPMEvent(container) {
    const div = document.createElement('div');
    div.className = 'midi-event bpm';
    div.style.border = '1px solid #333';
    div.style.padding = '0.5rem';
    div.style.marginBottom = '0.5rem';
    div.style.borderRadius = '4px';
    div.style.backgroundColor = '#443322';

    const titleRow = document.createElement('div');
    titleRow.style.display = 'flex';
    titleRow.style.justifyContent = 'space-between';
    titleRow.style.alignItems = 'center';
    titleRow.style.marginBottom = '0.5rem';

    const title = document.createElement('strong');
    title.textContent = '⚡ BPM Event';
    title.style.fontSize = '1rem';
    title.style.color = '#fff';

    const removeBtn = document.createElement('button');
    removeBtn.textContent = '✖';
    removeBtn.style.background = '#444';
    removeBtn.style.color = '#fff';
    removeBtn.style.border = 'none';
    removeBtn.style.borderRadius = '4px';
    removeBtn.style.padding = '0.3rem 0.6rem';
    removeBtn.style.cursor = 'pointer';
    removeBtn.onclick = () => container.removeChild(div);

    titleRow.appendChild(title);
    titleRow.appendChild(removeBtn);
    div.appendChild(titleRow);

    const separator = document.createElement('hr');
    separator.style.border = '0';
    separator.style.borderTop = '1px solid #444';
    separator.style.margin = '0.5rem 0';
    div.appendChild(separator);

    div.appendChild(createFeatureDropdown('tick'));
    div.appendChild(createFeatureDropdown('bpm'));

    container.appendChild(div);
}

function removeMIDIMappingChannel(container, blockToRemove) {
    container.removeChild(blockToRemove);

    // Re-label remaining channels
    const blocks = container.querySelectorAll('.midi-channel-block');
    blocks.forEach((block, index) => {
        const title = block.querySelector('h3');
        if (title) {
            title.textContent = `Channel ${index}`;
        }
    });
}

export function addMIDIMappingChannel() {
    const container = document.getElementById('midiMapper');
    const channelIndex = container.querySelectorAll('.midi-channel-block')?.length || 0;

    const block = document.createElement('div');
    block.className = 'midi-channel-block';
    block.style.border = '1px solid #555';
    block.style.borderRadius = '8px';
    block.style.padding = '1rem';
    block.style.marginBottom = '1rem';
    block.style.backgroundColor = '#1a1a1a';

    // Header with title and remove button
    const header = document.createElement('div');
    header.style.display = 'flex';
    header.style.justifyContent = 'space-between';
    header.style.alignItems = 'center';
    header.style.marginBottom = '0.5rem';

    const title = document.createElement('h3');
    title.textContent = `Channel ${channelIndex}`;
    title.style.margin = 0;
    title.style.fontSize = '1.2rem';
    title.style.color = '#ccc';

    const removeButton = document.createElement('button');
    removeButton.textContent = '✖';
    removeButton.style.width = '4rem';
    removeButton.style.backgroundColor = '#444';
    removeButton.style.color = '#fff';
    removeButton.style.border = 'none';
    removeButton.style.borderRadius = '4px';
    removeButton.style.padding = '0.3rem 0.6rem';
    removeButton.onclick = () => removeMIDIMappingChannel(container, block);

    header.appendChild(title);
    header.appendChild(removeButton);
    block.appendChild(header);

    // Event controls
    const eventsContainer = document.createElement('div');
    eventsContainer.className = 'midi-events';
    eventsContainer.style.padding = '0.5rem';
    eventsContainer.style.background = '#111';
    eventsContainer.style.borderRadius = '4px';
    eventsContainer.style.marginTop = '0.5rem';

    const buttonsContainer = document.createElement('div');
    buttonsContainer.style.display = 'flex';
    buttonsContainer.style.flexWrap = 'wrap';
    buttonsContainer.style.gap = '0.5rem';
    buttonsContainer.style.marginBottom = '0.5rem';

    buttonsContainer.appendChild(createEventButton('➕ Note Event', () => renderMIDINoteEvent(eventsContainer)));
    buttonsContainer.appendChild(createEventButton('➕ Time Signature', () => renderMIDITimeSignatureEvent(eventsContainer)));
    buttonsContainer.appendChild(createEventButton('➕ BPM Event', () => renderMIDIBPMEvent(eventsContainer)));

    block.appendChild(buttonsContainer);
    block.appendChild(eventsContainer);

    container.appendChild(block);
}

function renderMIDIToVisualizer(midiUint8Array) {
    const blob = new Blob([midiUint8Array], { type: 'audio/midi' });
    const url = URL.createObjectURL(blob);

    const player = document.getElementById('midiPlayer');
    const pianoVis = document.getElementById('pianoVis');
    const staffVis = document.getElementById('staffVis');

    player.src = url;
    pianoVis.src = url;
    staffVis.src = url;
}

export function hideMIDICanvas() {
    document.getElementById("midiCanvasWrapper").style.display = "none";
}

export function showMIDICanvas() {
    // Validate all feature dropdowns
    const unselected = featureDropdownElements.filter(select => !select.value || select.value === '');
    if (unselected.length > 0) {
        alert("Please select values for all mappings before visualizing.");
        return;
    }

    document.getElementById("midiCanvasWrapper").style.display = "block";
    constructMIDI();
}

let constructedMIDI = null; // stores last MIDI Uint8Array for export
function constructMIDI() {
    const ticksPerBeat = document.getElementById('midiTicksPerBeat').value;

    const smf = new JZZ.MIDI.SMF(1, ticksPerBeat);
    const channelBlocks = document.querySelectorAll('.midi-channel-block');

    channelBlocks.forEach((block, channelIndex) => {
        const track = new JZZ.MIDI.SMF.MTrk();
        let maxTick = 0;

        // Add default tempo and time signature
        track.add(0, JZZ.MIDI.smfTempo(500000));
        track.add(0, JZZ.MIDI.smfTimeSignature(4, 4, 24, 8));

        const eventsContainer = block.querySelector('.midi-events');
        const events = eventsContainer.querySelectorAll('.midi-event');

        events.forEach(event => {
            const inputs = event.querySelectorAll('select');
            const getInputPath = name => {
                const sel = Array.from(inputs).find(s => s.name === name);
                return sel?.value || null;
            };

            if (event.classList.contains('note')) {
                const tickPath = getInputPath('tick');
                const pitchPath = getInputPath('pitch');
                const velocityPath = getInputPath('velocity');
                const durationPath = getInputPath('duration');

                const ticks = getFeatureArray(tickPath);
                const pitches = getFeatureArray(pitchPath);
                const velocities = getFeatureArray(velocityPath);
                const durations = getFeatureArray(durationPath);

                const length = Math.min(ticks.length, pitches.length, velocities.length, durations.length);
                console.log("length: ", length);
                for (let i = 0; i < length; i++) {
                    const tick = ticks[i];
                    const pitch = pitches[i];
                    const velocity = velocities[i];
                    const duration = durations[i];

                    console.log(tick, pitch, velocity, duration);

                    track.add(tick, JZZ.MIDI.noteOn(channelIndex, pitch, velocity));
                    track.add(tick + duration, JZZ.MIDI.noteOff(channelIndex, pitch));
                    maxTick = Math.max(maxTick, tick + duration);
                }
            }

            if (event.classList.contains('bpm')) {
                const tickPath = getInputPath('tick');
                const bpmPath = getInputPath('bpm');

                const ticks = getFeatureArray(tickPath);
                const bpms = getFeatureArray(bpmPath);
                const len = Math.min(ticks.length, bpms.length);

                for (let i = 0; i < len; i++) {
                    const tick = ticks[i];
                    const bpm = bpms[i];
                    const mpqn = bpm > 0 ? Math.round(60000000 / bpm) : 500000;
                    track.add(tick, JZZ.MIDI.smfTempo(mpqn));
                    maxTick = Math.max(maxTick, tick);
                }
            }

            if (event.classList.contains('timesig')) {
                const tickPath = getInputPath('tick');
                const numPath = getInputPath('numerator');
                const denPath = getInputPath('denominator');

                const ticks = getFeatureArray(tickPath);
                const nums = getFeatureArray(numPath);
                const dens = getFeatureArray(denPath);
                const len = Math.min(ticks.length, nums.length, dens.length);

                for (let i = 0; i < len; i++) {
                    const tick = ticks[i];
                    const numerator = nums[i];
                    const denominator = dens[i];
                    track.add(tick, JZZ.MIDI.smfTimeSignature(numerator, denominator, 24, 8));
                    maxTick = Math.max(maxTick, tick);
                }
            }
        });
        console.log("Max tick: " + maxTick);
        track.add(maxTick + 1, JZZ.MIDI.smfEndOfTrack());
        smf.push(track);
    });
    const midiBytes = 'data:audio/midi;base64,' + JZZ.lib.toBase64(smf.dump());

    const byteCharacters = atob(midiBytes.split(',')[1]);
    const byteNumbers = new Array(byteCharacters.length);
    for (let i = 0; i < byteCharacters.length; i++) {
        byteNumbers[i] = byteCharacters.charCodeAt(i);
    }
    const byteArray = new Uint8Array(byteNumbers);

    renderMIDIToVisualizer(byteArray);
    constructedMIDI = byteArray;
    document.getElementById('btn_downloadMIDI').style.display = 'inline-block';
}

export function downloadMIDI() {
    if (!constructedMIDI) {
        alert("No MIDI available to download. Please visualize first.");
        return;
    }

    const blob = new Blob([constructedMIDI], { type: 'audio/midi' });
    const url = URL.createObjectURL(blob);

    const a = document.createElement('a');
    a.href = url;
    a.download = `${rootFeatureName}.mid`;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);

    URL.revokeObjectURL(url);
}

// --------------------------------------------------------------------------
// Running Instances
// --------------------------------------------------------------------------
let runningInstanceData = []; // indexed by node.id
let currentNodeEditing = null;

function openRIPopup(nodeId, label) {
    currentNodeEditing = nodeId;
    document.getElementById("popupFeatureLabel").textContent = label;
    const existing = runningInstanceData[nodeId] || [0, 0];
    document.getElementById("popupStartPoint").value = existing[0];
    document.getElementById("popupTransformShift").value = existing[1];
    document.getElementById("popupInstanceEditor").style.display = 'block';
}

export function closePopup() {
    document.getElementById("popupInstanceEditor").style.display = 'none';
    currentNodeEditing = null;
}

export function saveInstanceEdit() {
    const a = parseInt(document.getElementById("popupStartPoint").value);
    const b = parseInt(document.getElementById("popupTransformShift").value);
    if (!isNaN(a) && !isNaN(b)) {
        runningInstanceData[currentNodeEditing] = [a, b];
        updateRunningInstanceList();
    }
    closePopup();
}

function updateRunningInstanceList() {
    const result = runningInstanceData
        .map((v, i) => v ? `(${v[0]};${v[1]})` : null)
        .filter(v => v !== null);

    document.getElementById('executeRunningInstances').textContent = result.length > 0
        ? `[${result.join(',')}]`
        : '';
}

function clearRunningInstances() {
    runningInstanceData = [];
    updateRunningInstanceList();
}

// --------------------------------------------------------------------------
// Execute
// --------------------------------------------------------------------------
function drawFeatureTree(treeData) {
    const nodes = [];
    const edges = [];

    treeData.forEach((item, index) => {
        nodes.push({
            id: item.id,
            label: item.name.split('/').pop(),
            title: `${item.name}\n${item.scalar ? 'Scalar' : 'Composite'}`,
            color: item.scalar ? '#1e90ff' : '#ffffff',
            font: { color: item.scalar ? '#fff' : '#000' },
            shape: item.scalar ? 'ellipse' : 'box'
        });

        if (item.parent !== -1) {
            edges.push({ from: item.parent, to: item.id });
        }
    });

    const container = document.getElementById('treeContainer');
    const data = { nodes: new vis.DataSet(nodes), edges: new vis.DataSet(edges) };
    const options = {
        layout: { hierarchical: { direction: 'UD', sortMethod: 'directed' } },
        physics: false,
        edges: { color: '#aaa' },
        interaction: { hover: true }
    };

    const network = new vis.Network(container, data, options);

    network.on("click", function (params) {
        if (params.nodes.length > 0) {
            const nodeId = params.nodes[0];
            const nodeData = treeData.find(n => n.id === nodeId);
            if (nodeData) {
                openRIPopup(nodeId, nodeData.name);
            }
        }
    });

    document.getElementById('treeContainer').style.display = 'block';
    network.fit();
}

async function fetchFeatureDepthFirst(rootFeatureName) {
    clearRunningInstances();

    let nodeIdCounter = 0;
    const compositeList = [];
    const apiBase = window.location.origin;
    const stack = [{
        parent: -1,
        path: '',
        name: rootFeatureName,
        assignId: () => nodeIdCounter++
    }];

    while (stack.length > 0) {
        const { parent, path, name, assignId } = stack.pop();
        const id = assignId();

        try {
            const res = await fetch(`${apiBase}/feature/${name}`);
            if (!res.ok) throw new Error(`Failed to fetch ${name}`);
            const feature = await res.json();

            const fullPath = `${path}/${feature.name}`;
            const scalar = feature.dimensions.length === 0;

            compositeList.push({ parent, id, name: fullPath, scalar });
            runningInstanceData[id] = [0, 0];

            // Push children in reverse so they’re visited left-to-right
            for (let i = feature.dimensions.length - 1; i >= 0; i--) {
                const dim = feature.dimensions[i];
                stack.push({
                    parent: id,
                    path: fullPath,
                    name: dim.feature_name,
                    assignId: () => nodeIdCounter++
                });
            }
        } catch (e) {
            console.error(`Error fetching ${name}:, e`);
        }
    }

    updateRunningInstanceList();
    availableFeatureNames = compositeList
        .filter(entry => entry.scalar)
        .map(entry => entry.name);
    // Update all dropdowns
    featureDropdownElements.forEach(populateFeatureDropdown);

    return compositeList;
}

export async function fetchBeforeExecute() {
    rootFeatureName = document.getElementById('executeName').value.trim();
    if (!rootFeatureName) {
        alert("Please provide a feature name.");
        return;
    }

    const features = await fetchFeatureDepthFirst(rootFeatureName);

    drawFeatureTree(features);
}

export async function execute() {
    const name = document.getElementById('executeName').value.trim();
    const N = document.getElementById('executeN').value.trim();
    const runningInstances = document.getElementById('executeRunningInstances').textContent.trim();

    const codeDiv = document.getElementById('executeCode');
    const bodyDiv = document.getElementById('executeBody');
    if (!name) {
        alert("Contract name is required.");
        return;
    }
    const apiBase = window.location.origin;
    const url = runningInstances === '' ? `${apiBase}/execute/${name}/${N}` : `${apiBase}/execute/${name}/${N}/${runningInstances}`;
    try {
        const res = await requestWithRefresh(url,
            {
                method: 'GET',
                headers: {
                    'Content-Type': 'application/json'
                },
                credentials: 'include'
            }
        );
        const text = await res.text();
        codeDiv.textContent = res.status;
        bodyDiv.textContent = formatJSON(text);
        // Store feature data for MIDI use
        const parsed = JSON.parse(text);
        if (Array.isArray(parsed)) {
            executeResultData = parsed;
        }
    } catch (error) {
        codeDiv.textContent = 'Error';
        bodyDiv.textContent = error.message;
    }
}

// --------------------------------------------------------------------------
// Feature
// --------------------------------------------------------------------------
export async function updateFeatureRequestPreview() {
    const json = constructStructuredFeature();
    document.getElementById('POST_featureRequestBody').textContent = json;
}

export function clearDimensions() {
    const container = document.getElementById('dimensionsContainer');
    container.innerHTML = '';
    updateFeatureRequestPreview();
}

export function addDimension() {
    const container = document.getElementById('dimensionsContainer');
    const index = container.children.length;

    const dim = document.createElement('div');
    dim.className = 'dimension';

    const fieldset = document.createElement('fieldset');
    fieldset.style.border = '1px solid #555';
    fieldset.style.padding = '1rem';
    fieldset.style.marginBottom = '0.5rem';

    const legend = document.createElement('legend');
    legend.textContent = `Dimension ${index + 1}`;

    // Create row container
    const row = document.createElement('div');
    row.style.display = 'flex';
    row.style.gap = '1rem';
    row.style.alignItems = 'flex-start';

    // Column feature name
    const nameCol = document.createElement('div');
    nameCol.style.flex = '1';
    nameCol.style.marginLeft = '1rem';
    nameCol.style.marginRight = '1rem';

    const label = document.createElement('label');
    label.textContent = 'Feature Name';

    const input = document.createElement('input');
    input.type = 'text';
    input.className = 'dimension-feature-name';
    input.placeholder = 'pitch';
    input.addEventListener('input', updateFeatureRequestPreview);

    nameCol.appendChild(label);
    nameCol.appendChild(input);

    const transformationsDiv = document.createElement('div');
    transformationsDiv.className = 'transformations';

    const addBtn = document.createElement('button');
    addBtn.type = 'button';
    addBtn.textContent = '➕ Add transformation';
    addBtn.addEventListener('click', () => addTransformation(addBtn));

    fieldset.appendChild(legend);

    // Assemble row
    row.appendChild(nameCol);
    fieldset.appendChild(row);

    fieldset.appendChild(transformationsDiv);
    fieldset.appendChild(addBtn);

    dim.appendChild(fieldset);
    container.appendChild(dim);

    updateFeatureRequestPreview();
}

export function addTransformation(button) {
    const container = button.previousElementSibling;

    const t = document.createElement('div');
    t.style.marginBottom = '1rem';

    const hr = document.createElement('hr');
    t.appendChild(hr);

    // Create row container
    const row = document.createElement('div');
    row.style.display = 'flex';
    row.style.gap = '1rem';
    row.style.alignItems = 'flex-start';

    // Column 1: name
    const nameCol = document.createElement('div');
    nameCol.style.flex = '1';
    nameCol.style.marginLeft = '1rem';
    nameCol.style.marginRight = '1rem';

    const nameLabel = document.createElement('label');
    nameLabel.textContent = 'Transformation name';

    const nameInput = document.createElement('input');
    nameInput.type = 'text';
    nameInput.className = 'transformation-name';
    nameInput.placeholder = 'add';
    nameInput.addEventListener('input', updateFeatureRequestPreview);

    nameCol.appendChild(nameLabel);
    nameCol.appendChild(nameInput);

    // Column 2: args
    const argsCol = document.createElement('div');
    argsCol.style.flex = '1';
    argsCol.style.marginLeft = '1rem';
    argsCol.style.marginRight = '1rem';

    const argsLabel = document.createElement('label');
    argsLabel.textContent = 'args (comma-separated)';

    const argsInput = document.createElement('input');
    argsInput.type = 'text';
    argsInput.className = 'transformation-args';
    argsInput.placeholder = '...';
    argsInput.addEventListener('input', updateFeatureRequestPreview);

    argsCol.appendChild(argsLabel);
    argsCol.appendChild(argsInput);

    // Assemble row
    row.appendChild(nameCol);
    row.appendChild(argsCol);
    t.appendChild(row);

    container.appendChild(t);
    updateFeatureRequestPreview();
}

function constructStructuredFeature() {
    const name = document.getElementById('in_featureName').value.trim();
    const dimensions = [];

    document.querySelectorAll('.dimension').forEach(dimEl => {
        const feature_name = dimEl.querySelector('.dimension-feature-name').value.trim();
        const transformations = [];
        dimEl.querySelectorAll('.transformations > div').forEach(tEl => {
            const tname = tEl.querySelector('.transformation-name').value.trim();
            const targs = tEl.querySelector('.transformation-args').value.trim().split(',').map(x => parseInt(x.trim())).filter(x => !isNaN(x));
            if (tname) {
                transformations.push({ name: tname, args: targs });
            }
        });
        if (feature_name) {
            dimensions.push({ feature_name, transformations });
        }
    });

    return JSON.stringify({ name, dimensions }, null, 2);
}

function populateStructuredFeature(jsonOrObject) {
    const data = typeof jsonOrObject === 'string' ? JSON.parse(jsonOrObject) : jsonOrObject;

    // Set name field
    document.getElementById('in_featureName').value = data.name || '';

    // Clear any existing dimensions
    clearDimensions();

    const container = document.getElementById('dimensionsContainer');
    data.dimensions?.forEach((dim, dimIndex) => {
        addDimension(); // creates and appends a new .dimension block

        const dimEl = container.children[dimIndex];

        // Set feature name
        dimEl.querySelector('.dimension-feature-name').value = dim.feature_name || '';

        const transformationsDiv = dimEl.querySelector('.transformations');

        dim.transformations?.forEach(t => {
            // Add a new transformation
            const addBtn = dimEl.querySelector('button'); // the addTransformation button
            addTransformation(addBtn);

            const tEl = transformationsDiv.lastElementChild;

            // Set transformation name
            tEl.querySelector('.transformation-name').value = t.name || '';

            // Set transformation args
            tEl.querySelector('.transformation-args').value = Array.isArray(t.args)
                ? t.args.join(', ')
                : '';
        });
    });

    updateFeatureRequestPreview(); // trigger preview update
}

export async function sendStructuredFeature() {
    const requestBody = constructStructuredFeature();

    const responseCodeDiv = document.getElementById('POST_featureResponseCode');
    const responseBodyDiv = document.getElementById('POST_featureResponseBody');

    try {
        const apiBase = window.location.origin;
        const res = await requestWithRefresh(`${apiBase}/feature`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: requestBody,
            credentials: 'include',
        });
        const text = await res.text();
        responseCodeDiv.textContent = res.status;
        responseBodyDiv.textContent = formatJSON(text);
    } catch (error) {
        responseCodeDiv.textContent = 'Error';
        responseBodyDiv.textContent = error.message;
    }
}

export async function getFeature() {
    const name = document.getElementById('in_featureName').value.trim();
    const address = document.getElementById('GET_featureAddress').value.trim();

    const responseCodeDiv = document.getElementById('GET_featureResponseCode');
    const responseBodyDiv = document.getElementById('GET_featureResponseBody');
    if (!name) {
        alert("Feature name is required.");
        return;
    }
    const apiBase = window.location.origin;
    const url = `${apiBase}/feature/${name}${address ? `/${address}` : ''}`;
    try {
        const res = await fetch(url);
        const text = await res.text();
        responseCodeDiv.textContent = res.status;
        responseBodyDiv.textContent = formatJSON(text);
        populateStructuredFeature(JSON.parse(text));
    } catch (error) {
        responseCodeDiv.textContent = 'Error';
        responseBodyDiv.textContent = error.message;
    }
}
// --------------------------------------------------------------------------
// Transformation
// --------------------------------------------------------------------------
export async function getTransformation() {
    const name = document.getElementById('in_transformationName').value.trim();
    const address = document.getElementById('GET_transformationAddress').value.trim();

    const responseCodeDiv = document.getElementById('GET_transformationResponseCode');
    const responseBodyDiv = document.getElementById('GET_transformationResponseBody');

    if (!name) {
        alert("Transformation name is required.");
        return;
    }

    const apiBase = window.location.origin;
    const url = `${apiBase}/transformation/${name}${address ? `/${address}` : ''}`;
    try {
        const res = await fetch(url);
        const text = await res.text();
        responseCodeDiv.textContent = res.status;
        responseBodyDiv.textContent = formatJSON(text);
        populateStructuredTransformation(JSON.parse(text));
    } catch (error) {
        responseCodeDiv.textContent = 'Error';
        responseBodyDiv.textContent = error.message;
    }
}

export function updateTransformationPreview() {
    const name = document.getElementById('in_transformationName').value.trim();
    const sol_src = document.getElementById('POST_transformationCode').value.trim();
    const obj = { name, sol_src };
    document.getElementById('POST_transformationRequestBody').textContent = JSON.stringify(obj, null, 2);
}

export function populateStructuredTransformation(jsonOrObject) {
    const data = typeof jsonOrObject === 'string' ? JSON.parse(jsonOrObject) : jsonOrObject;

    const nameInput = document.getElementById('in_transformationName');
    const codeInput = document.getElementById('POST_transformationCode');
    const requestBodyDiv = document.getElementById('POST_transformationRequestBody');

    if (!data || typeof data !== 'object') {
        console.warn("Invalid transformation data:", data);
        return;
    }

    nameInput.value = data.name || '';
    codeInput.value = data.sol_src || '';

    updateTransformationPreview(); // trigger live preview rendering
}

export async function sendStructuredTransformation() {
    const name = document.getElementById('in_transformationName').value.trim();
    const sol_src = document.getElementById('POST_transformationCode').value.trim();

    const responseCodeDiv = document.getElementById('POST_transformationResponseCode');
    const responseBodyDiv = document.getElementById('POST_transformationResponseBody');

    if (!name) {
        alert("Transformation name is required.");
        return;
    }

    try {
        const apiBase = window.location.origin;
        const res = await requestWithRefresh(`${apiBase}/transformation`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ name, sol_src }),
            credentials: 'include'
        });
        const text = await res.text();
        responseCodeDiv.textContent = res.status;
        responseBodyDiv.textContent = formatJSON(text);
    } catch (error) {
        responseCodeDiv.textContent = 'Error';
        responseBodyDiv.textContent = error.message;
    }
}

// --------------------------------------------------------------------------
// Version
// --------------------------------------------------------------------------
export async function fetchVersionInfo() {
    const versionDiv = document.getElementById('versionInfo');
    try {
        const res = await fetch(`${window.location.origin}/version`);
        const data = await res.json();
        versionDiv.textContent = `Version ${data.version} (Built: ${data.build_timestamp})`;
    } catch (err) {
        versionDiv.textContent = "⚠️ Failed to fetch version info";
        console.error("Version fetch failed:", err);
    }
}

// --------------------------------------------------------------------------
// Account
// --------------------------------------------------------------------------
let currentAccountPage = 0;
const accountPageSize = 10;
let totalAccountFeaturePages = 0;
let totalAccountTransformationPages = 0;

export function nextPage()
{
    if(currentAccountPage < Math.max(totalAccountFeaturePages, totalAccountTransformationPages))
    {
        currentAccountPage += 1;
        fetchAccountResources();
    }
}

export function prevPage()
{
    if (currentAccountPage > 0) {
        currentAccountPage--;
        fetchAccountResources();
    }
}

export async function fetchAccountResources() {
    const address = document.getElementById('accountAddressInput').value.trim();
    const featuresDiv = document.getElementById('accountFeaturesList');
    const transformationsDiv = document.getElementById('accountTransformationsList');

    featuresDiv.textContent = 'Loading...';
    transformationsDiv.textContent = 'Loading...';

    if (!address) {
        alert("Address is required.");
        featuresDiv.textContent = '❌ Invalid address';
        transformationsDiv.textContent = '';
        return;
    }

    try {
        const apiBase = window.location.origin;
        const res = await requestWithRefresh(`${apiBase}/account/${address}?limit=${accountPageSize}&page=${currentAccountPage}`, {
            method: 'GET',
            headers: { 'Content-Type': 'application/json' }
        });

        const data = await res.json();

        availableFeatureNames = data.owned_features;
        featureDropdownElements.forEach(populateFeatureDropdown);

        featuresDiv.innerHTML = data.owned_features?.length
            ? data.owned_features.map((name, i) => {
                const bg = i % 2 === 0 ? '#1e1e1e' : '#2a2a2a';
                return `<div style="padding: 0.5rem; border: 1px solid #3333; border-radius: 4px; margin-bottom: 0.3rem; background: ${bg};"><code>${name}</code></div>`;
            }).join('')
            : '(none)';

        transformationsDiv.innerHTML = data.owned_transformations?.length
            ? data.owned_transformations.map((name, i) => {
                const bg = i % 2 === 0 ? '#1e1e1e' : '#2a2a2a';
                return `<div style="padding: 0.5rem; border: 1px solid #3333; border-radius: 4px; margin-bottom: 0.3rem; background: ${bg};"><code>${name}</code></div>`;
            }).join('')
            : '(none)';

        // Compute total pages based on backend totals
        totalAccountFeaturePages = Math.ceil((data.total_features ?? 0) / accountPageSize);
        totalAccountTransformationPages = Math.ceil((data.total_transformations ?? 0) / accountPageSize);

        // Show page number as: Page X of Y
        document.getElementById('accountPageLabel').textContent =
            `Page ${currentAccountPage + 1} of ${Math.max(totalAccountFeaturePages, totalAccountTransformationPages)}`;

        // Disable next if on last page
        const isLastPage = currentAccountPage >= Math.max(totalAccountFeaturePages, totalAccountTransformationPages) - 1;
        document.getElementById('btn_prevAccountPage').disabled = currentAccountPage === 0;
        document.getElementById('btn_nextAccountPage').disabled = isLastPage;

    } catch (err) {
        featuresDiv.textContent = '❌ Failed to fetch account data';
        transformationsDiv.textContent = err.message;
    }
}

// --------------------------------------------------------------------------
// Login
// --------------------------------------------------------------------------

// MetaMask login functionality
export async function loginWithMetaMask() 
{
    const loginStatusDiv = document.getElementById('loginStatus');

    if (typeof window.ethereum === 'undefined') {
        alert("MetaMask is not installed!");
        return;
    }
    try {
        const [address] = await window.ethereum.request({ method: 'eth_requestAccounts' });
        const apiBase = window.location.origin;
        const nonceRes = await fetch(`${apiBase}/nonce/` + address);
        const { nonce } = await nonceRes.json();

        const message = `Login nonce: ${nonce}`;
        const signature = await window.ethereum.request({
            method: 'personal_sign',
            params: [message, address],
        });

        const authRes = await fetch(`${apiBase}/auth`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ address, signature, message }),
        });

        const result = await authRes.json();

        if (result.access_token) {
            loginStatusDiv.innerHTML = `<p style="color: green;">✅ Authenticated as ${address}</p>`;
        }
        else {
            loginStatusDiv.innerHTML = `<p style="color: red;">❌ Authentication failed</p>`;
        }
    } catch (err) {
        console.error(err);
        loginStatusDiv.innerHTML = `<p style="color: red;">⚠️ Error: ${err.message}</p>`;
    }
}

// --------------------------------------------------------------------------
// Utils
// --------------------------------------------------------------------------

export function copyTextFromElement(elementId) {
    const el = document.getElementById(elementId);
    if (!el) return;
  
    const text = el.innerText || el.textContent;
    navigator.clipboard.writeText(text)
      .then(() => {
      })
      .catch(err => {
        alert("Failed to copy: " + err);
      });
  }

function getFeatureArray(path) {
    const entry = executeResultData.find(f => f.feature_path === path);
    return entry?.data ?? [];
}

// Function to format the JSON response nicely
function formatJSON(text) {
    try {
        const json = JSON.parse(text);
        return JSON.stringify(json, null, 2); // Pretty-print with 2 spaces
    } catch (e) {
        return text; // If not valid JSON, return as it is
    }
}

function hexToRgba(hex, alpha = 1.0) {
    const r = parseInt(hex.substr(1, 2), 16);
    const g = parseInt(hex.substr(3, 2), 16);
    const b = parseInt(hex.substr(5, 2), 16);
    return `rgba(${r},${g},${b},${alpha})`;
}

export async function requestWithRefresh(url, options = {}) {
    // always include cookies
    options.credentials = 'include';

    // first attempt
    let res = await fetch(url, options);
    if (res.status !== 401) {
        return res;
    }

    // 401 → try to refresh
    const apiBase = window.location.origin;
    const refreshRes = await fetch(`${apiBase}/refresh`, {
        method: 'POST',
        credentials: 'include'
    });
    if (refreshRes.ok) {
        // retry original request once
        res = await fetch(url, options);
    }
    return res;
}
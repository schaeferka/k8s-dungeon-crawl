let activeMonstersData = [];
let deadMonstersData = [];
let allMonstersData = [];

async function fetchMonsterData() {
    /**
     * Fetches data for live monsters, dead monsters, and all monsters from the server.
     * This function sends requests to the relevant endpoints and updates the data arrays 
     * accordingly.
     */
    try {
        // Fetch active monsters data
        const activeMonstersResponse = await fetch('/monsters/data');
        activeMonstersData = await activeMonstersResponse.json();

        // Fetch dead monsters data
        const deadMonstersResponse = await fetch('/monsters/dead');
        deadMonstersData = await deadMonstersResponse.json();

        // Fetch all monsters data
        const allMonstersResponse = await fetch('/monsters/all');
        allMonstersData = await allMonstersResponse.json();

        // Apply any filters that have been set
        applyFilters();
    } catch (error) {
        console.error('Error fetching monster data:', error);
    }
}

function applyFilters() {
    /**
     * Applies filters based on type and depth input values and updates the displayed tables.
     * Filters the monster data arrays for live, dead, and all monsters.
     */
    const typeFilter = document.getElementById('monster-type-filter').value.toLowerCase();
    const depthFilter = document.getElementById('depth-filter').value;

    // Filter data based on the inputs
    const filteredActiveMonsters = activeMonstersData.filter(monster => {
        const matchesType = monster.type.toLowerCase().includes(typeFilter);
        const matchesDepth = depthFilter ? monster.depth == parseInt(depthFilter) : true;
        return matchesType && matchesDepth;
    });

    const filteredDeadMonsters = deadMonstersData.filter(monster => {
        const matchesType = monster.type.toLowerCase().includes(typeFilter);
        const matchesDepth = depthFilter ? monster.depth == parseInt(depthFilter) : true;
        return matchesType && matchesDepth;
    });

    const filteredAllMonsters = allMonstersData.filter(monster => {
        const matchesType = monster.type.toLowerCase().includes(typeFilter);
        const matchesDepth = depthFilter ? monster.depth == parseInt(depthFilter) : true;
        return matchesType && matchesDepth;
    });

    // Update tables with filtered data
    updateTables(filteredActiveMonsters, filteredDeadMonsters, filteredAllMonsters);
}

function resetFilters() {
    /**
     * Resets all filters and updates the tables with full monster data.
     */
    // Clear filter inputs and reset tables
    document.getElementById('monster-type-filter').value = '';
    document.getElementById('depth-filter').value = '';
    updateTables(activeMonstersData, deadMonstersData, allMonstersData); // Reset tables with full data
}

function updateTables(activeMonsters, deadMonsters, allMonsters) {
    /**
     * Updates the tables for live monsters, dead monsters, and all monsters with the provided
     * data.
     * 
     * @param {Array} activeMonsters - List of live monsters to display.
     * @param {Array} deadMonsters - List of dead monsters to display.
     * @param {Array} allMonsters - List of all monsters (live and dead) to display.
     */
    const activeTableBody = document.getElementById('active-monsters-body');
    const deadTableBody = document.getElementById('dead-monsters-body');
    const allTableBody = document.getElementById('all-monsters-body');

    activeTableBody.innerHTML = '';
    deadTableBody.innerHTML = '';
    allTableBody.innerHTML = '';

    // Update active monsters table
    activeMonsters.forEach(monster => {
        const row = document.createElement('tr');

        // Create monster name link
        const nameCell = document.createElement('td');
        nameCell.classList.add('text-center');
        const link = document.createElement('a');
        link.href = 'http://localhost:8080/' + monster.name; 
        link.textContent = monster.name || 'Unknown'; 
        link.classList.add('text-blue-500', 'hover:text-blue-700', 'transition-colors', 'no-underline');
        nameCell.appendChild(link);

        // Create other monster data cells
        const hpCell = document.createElement('td');
        hpCell.classList.add('text-center');
        hpCell.textContent = monster.hp || '-';

        const maxHpCell = document.createElement('td');
        maxHpCell.classList.add('text-center');
        maxHpCell.textContent = monster.max_hp || '-';

        const depthCell = document.createElement('td');
        depthCell.classList.add('text-center');
        depthCell.textContent = monster.depth || '-';

        const defenseCell = document.createElement('td');
        defenseCell.classList.add('text-center');
        defenseCell.textContent = monster.defense || '-';

        const accuracyCell = document.createElement('td');
        accuracyCell.classList.add('text-center');
        accuracyCell.textContent = monster.accuracy || '-';

        const damageCell = document.createElement('td');
        damageCell.classList.add('text-center');
        damageCell.textContent = `${monster.damage_min || 'N/A'} - ${monster.damage_max || 'N/A'}`;

        const turnsRegenCell = document.createElement('td');
        turnsRegenCell.classList.add('text-center');
        turnsRegenCell.textContent = monster.turns_between_regen || '-';

        const movementSpeedCell = document.createElement('td');
        movementSpeedCell.classList.add('text-center');
        movementSpeedCell.textContent = monster.movement_speed || '-';

        const attackSpeedCell = document.createElement('td');
        attackSpeedCell.classList.add('text-center');
        attackSpeedCell.textContent = monster.attack_speed || '-';

        const idCell = document.createElement('td');
        idCell.classList.add('text-center');
        idCell.textContent = monster.id || '-';

        const typeCell = document.createElement('td');
        typeCell.classList.add('text-center');
        typeCell.textContent = monster.type || '-';

        // Append all cells to the row
        row.appendChild(nameCell);
        row.appendChild(hpCell);
        row.appendChild(maxHpCell);
        row.appendChild(depthCell);
        row.appendChild(defenseCell);
        row.appendChild(accuracyCell);
        row.appendChild(damageCell);
        row.appendChild(turnsRegenCell);
        row.appendChild(movementSpeedCell);
        row.appendChild(attackSpeedCell);
        row.appendChild(idCell);
        row.appendChild(typeCell);

        // Append the row to the table body
        activeTableBody.appendChild(row);
    });

    // Update dead monsters table (similar to the above method)
    deadMonsters.forEach(monster => {
        const row = document.createElement('tr');

        const nameCell = document.createElement('td');
        nameCell.classList.add('text-center');
        nameCell.textContent = monster.name || '';

        const depthCell = document.createElement('td');
        depthCell.classList.add('text-center');
        depthCell.textContent = monster.depth || '';

        const hpCell = document.createElement('td');
        hpCell.classList.add('text-center');
        hpCell.textContent = `${monster.hp || '0'} / ${monster.max_hp || ''}`;

        const idCell = document.createElement('td');
        idCell.classList.add('text-center');
        idCell.textContent = monster.id || '';

        row.appendChild(nameCell);
        row.appendChild(depthCell);
        row.appendChild(hpCell);
        row.appendChild(idCell);
        deadTableBody.appendChild(row);
    });

    // Update all monsters table (similar to the above method)
    allMonsters.forEach(monster => {
        const row = document.createElement('tr');

        // Apply a red background if the monster is dead
        if (monster.is_dead) {
            row.classList.add('bg-red-200'); // Tailwind class for light red background
        }

        const nameCell = document.createElement('td');
        nameCell.classList.add('text-center');
        nameCell.textContent = monster.name || '-';

        const hpCell = document.createElement('td');
        hpCell.classList.add('text-center');
        hpCell.textContent = monster.is_dead ? '0' : monster.hp || '';  // Set to 0 if dead, else show HP

        const maxHpCell = document.createElement('td');
        maxHpCell.classList.add('text-center');
        maxHpCell.textContent = monster.max_hp || '-';

        const depthCell = document.createElement('td');
        depthCell.classList.add('text-center');
        depthCell.textContent = monster.depth || '-';

        const defenseCell = document.createElement('td');
        defenseCell.classList.add('text-center');
        defenseCell.textContent = monster.defense || '-';

        const accuracyCell = document.createElement('td');
        accuracyCell.classList.add('text-center');
        accuracyCell.textContent = monster.accuracy || '-';

        const damageCell = document.createElement('td');
        damageCell.classList.add('text-center');
        damageCell.textContent = `${monster.damage_min || 'N/A'} - ${monster.damage_max || 'N/A'}`;

        const movementSpeedCell = document.createElement('td');
        movementSpeedCell.classList.add('text-center');
        movementSpeedCell.textContent = monster.movement_speed || '-';

        const attackSpeedCell = document.createElement('td');
        attackSpeedCell.classList.add('text-center');
        attackSpeedCell.textContent = monster.attack_speed || '-';

        const idCell = document.createElement('td');
        idCell.classList.add('text-center');
        idCell.textContent = monster.id || '-';

        const typeCell = document.createElement('td');
        typeCell.classList.add('text-center');
        typeCell.textContent = monster.type || '-';

        row.appendChild(nameCell);
        row.appendChild(hpCell);
        row.appendChild(maxHpCell);
        row.appendChild(depthCell);
        row.appendChild(defenseCell);
        row.appendChild(accuracyCell);
        row.appendChild(damageCell);
        row.appendChild(movementSpeedCell);
        row.appendChild(attackSpeedCell);
        row.appendChild(idCell);
        row.appendChild(typeCell);
        allTableBody.appendChild(row);
    });
}

setInterval(fetchMonsterData, 1000);

// Fetch data initially
window.onload = fetchMonsterData;

// Add event listeners to buttons
document.getElementById('apply-filters').addEventListener('click', applyFilters);
document.getElementById('reset-filters').addEventListener('click', resetFilters);

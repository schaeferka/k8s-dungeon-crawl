let activeMonstersData = [];
let deadMonstersData = [];
let allMonstersData = [];
let adminKillsData = [];

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

async function fetchAdminKillsData() {
    /**
     * Fetches data for admin kills from the server.
     * This function sends a request to the relevant endpoint and updates the data array accordingly.
     */
    try {
        // Fetch admin kills data
        const adminKillsResponse = await fetch('/monsters/admin-kills');
        adminKillsData = await adminKillsResponse.json();
        applyFilters(); // Apply filters after fetching data
    } catch (error) {
        console.error('Error fetching admin kills data:', error);
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

    const filteredAdminKills = adminKillsData.filter(kill => {
        const matchesType = kill.monster_name.toLowerCase().includes(typeFilter);
        const matchesDepth = depthFilter ? kill.depth == parseInt(depthFilter) : true;
        return matchesType && matchesDepth;
    });

    // Update tables with filtered data
    updateTables(filteredActiveMonsters, filteredDeadMonsters, filteredAllMonsters);
    updateAdminKillsTable(filteredAdminKills);
}   

function resetFilters() {
    /**
     * Resets all filters and updates the tables with full monster data.
     */
    // Clear filter inputs and reset tables
    document.getElementById('monster-type-filter').value = '';
    document.getElementById('depth-filter').value = '';
    updateTables(activeMonstersData, deadMonstersData, allMonstersData, adminKillsData); // Reset tables with full data
}

function createTableCell(content, className = 'text-center') {
    const cell = document.createElement('td');
    cell.classList.add(className);
    cell.textContent = content || '-';
    return cell;
}

function createMonsterRow(monster, isDead = false, isActive = false) {
    const row = document.createElement('tr');
    if (isDead) {
        row.classList.add('bg-red-200');
    }

    const nameCell = document.createElement('td');
    nameCell.classList.add('text-center');
    if (isActive) {
        const link = document.createElement('a');
        link.href = 'http://localhost:8080/' + monster.name;
        link.textContent = monster.name || 'Unknown';
        link.classList.add('text-blue-500', 'hover:text-blue-700', 'transition-colors', 'no-underline');
        nameCell.appendChild(link);
    } else {
        nameCell.textContent = monster.name || 'Unknown';
    }

    row.appendChild(nameCell);
    row.appendChild(createTableCell(monster.pod_name));
    row.appendChild(createTableCell(monster.hp));
    row.appendChild(createTableCell(monster.max_hp));
    row.appendChild(createTableCell(monster.depth));
    row.appendChild(createTableCell(monster.defense));
    row.appendChild(createTableCell(monster.accuracy));
    row.appendChild(createTableCell(`${monster.damage_min || 'N/A'} - ${monster.damage_max || 'N/A'}`));
    row.appendChild(createTableCell(monster.turns_between_regen));
    row.appendChild(createTableCell(monster.movement_speed));
    row.appendChild(createTableCell(monster.attack_speed));
    row.appendChild(createTableCell(monster.id));
    row.appendChild(createTableCell(monster.type));

    return row;
}

function createAdminKillRow(kill) {
    const row = document.createElement('tr');
    row.appendChild(createTableCell(kill.monster_name));
    row.appendChild(createTableCell(kill.pod_name));
    row.appendChild(createTableCell(kill.depth));
    row.appendChild(createTableCell(kill.namespace));
    row.appendChild(createTableCell(new Date(kill.timestamp).toUTCString())); // Format the timestamp
    row.appendChild(createTableCell(kill.monster_id));
    return row;
}

function updateTables(activeMonsters, deadMonsters, allMonsters) {
    /**
     * Updates the tables for live monsters, dead monsters, all monsters, and admin kills with the provided
     * data.
     * 
     * @param {Array} activeMonsters - List of live monsters to display.
     * @param {Array} deadMonsters - List of dead monsters to display.
     * @param {Array} allMonsters - List of all monsters (live and dead) to display.
     * 
     */
    const activeTableBody = document.getElementById('active-monsters-body');
    const deadTableBody = document.getElementById('dead-monsters-body');
    const allTableBody = document.getElementById('all-monsters-body');

    activeTableBody.innerHTML = '';
    deadTableBody.innerHTML = '';
    allTableBody.innerHTML = '';

    // Update active monsters table
    activeMonsters.forEach(monster => {
        activeTableBody.appendChild(createMonsterRow(monster, false, true));
    });

    // Update dead monsters table (similar to the above method)
    deadMonsters.forEach(monster => {
        const row = document.createElement('tr');
        row.appendChild(createTableCell(monster.name));
        row.appendChild(createTableCell(monster.pod_name));
        row.appendChild(createTableCell(monster.depth));
        row.appendChild(createTableCell(`${monster.hp || '0'} / ${monster.max_hp || ''}`));
        row.appendChild(createTableCell(monster.death_timestamp));
        row.appendChild(createTableCell(monster.id));
        deadTableBody.appendChild(row);
    });

    // Update all monsters table (similar to the above method)
    allMonsters.forEach(monster => {
        allTableBody.appendChild(createMonsterRow(monster, monster.is_dead, false));
    });
}

function updateAdminKillsTable(kills) {
    /**
     * Updates the admin kills table with the provided data.
     * 
     * @param {Array} kills - List of admin kills to display.
     */
    const tableBody = document.getElementById('admin-kills-body');
    tableBody.innerHTML = '';

    kills.forEach(kill => {
        tableBody.appendChild(createAdminKillRow(kill));
    });
}

setInterval(fetchMonsterData, 1000);
setInterval(fetchAdminKillsData, 1000);

// Fetch data initially
window.onload = () => {
    fetchMonsterData();
    fetchAdminKillsData();
};

// Add event listeners to buttons
document.getElementById('apply-filters').addEventListener('click', applyFilters);
document.getElementById('reset-filters').addEventListener('click', resetFilters);
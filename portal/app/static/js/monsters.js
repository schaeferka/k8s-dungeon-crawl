let activeMonstersData = [];
let deadMonstersData = [];
let allMonstersData = [];
let adminKillsData = [];

async function fetchMonsterData() {
    /**
     * Fetches data for live monsters, dead monsters, all monsters, and admin kills from the server.
     * Updates the corresponding data arrays and tables.
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

        // Fetch admin kills data
        const adminKillsResponse = await fetch('/monsters/admin-kills');
        adminKillsData = await adminKillsResponse.json();

        // Update tables with fetched data
        applyFilters();
    } catch (error) {
        console.error('Error fetching monster data:', error);
    }
}

function applyFilters() {
    /**
     * Applies filters based on type and depth input values and updates the displayed tables.
     * Filters the monster data arrays for live, dead, all monsters, and admin kills.
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

    const filteredAdminKills = adminKillsData.filter(adminKill => {
        const matchesType = adminKill.monsterName.toLowerCase().includes(typeFilter);
        const matchesDepth = depthFilter ? adminKill.depth == parseInt(depthFilter) : true;
        return matchesType && matchesDepth;
    });

    // Update tables with filtered data
    updateTables(filteredActiveMonsters, filteredDeadMonsters, filteredAllMonsters, filteredAdminKills);
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

function createRow(data, isDead = false, isActive = false, isAdminKill = false) {
    const row = document.createElement('tr');
    if (isDead) {
        row.classList.add('bg-red-200');
    }

    const nameCell = document.createElement('td');
    nameCell.classList.add('text-center');
    if (isActive) {
        const link = document.createElement('a');
        link.href = 'http://localhost:8080/' + data.name;
        link.textContent = data.name || 'Unknown';
        link.classList.add('text-blue-500', 'hover:text-blue-700', 'transition-colors', 'no-underline');
        nameCell.appendChild(link);
    } else {
        nameCell.textContent = isAdminKill ? data.monsterName || 'Unknown' : data.name || 'Unknown';
    }

    row.appendChild(nameCell);
    row.appendChild(createTableCell(isAdminKill ? data.depth : data.depth));
    row.appendChild(createTableCell(isAdminKill ? data.hp : `${data.hp || '0'} / ${data.max_hp || ''}`));
    row.appendChild(createTableCell(isAdminKill ? data.monsterID : data.id));

    return row;
}

function updateTables(activeMonsters, deadMonsters, allMonsters, adminKills) {
    /**
     * Updates the tables for live monsters, dead monsters, all monsters, and admin kills with the provided data.
     * 
     * @param {Array} activeMonsters - List of live monsters to display.
     * @param {Array} deadMonsters - List of dead monsters to display.
     * @param {Array} allMonsters - List of all monsters (live and dead) to display.
     * @param {Array} adminKills - List of admin kills to display.
     */
    const activeTableBody = document.getElementById('active-monsters-body');
    const deadTableBody = document.getElementById('dead-monsters-body');
    const allTableBody = document.getElementById('all-monsters-body');
    const adminKillsTableBody = document.getElementById('admin-kills-body');

    activeTableBody.innerHTML = '';
    deadTableBody.innerHTML = '';
    allTableBody.innerHTML = '';
    adminKillsTableBody.innerHTML = '';

    // Update active monsters table
    activeMonsters.forEach(monster => {
        activeTableBody.appendChild(createRow(monster, false, true));
    });

    // Update dead monsters table
    deadMonsters.forEach(monster => {
        deadTableBody.appendChild(createRow(monster, true));
    });

    // Update all monsters table
    allMonsters.forEach(monster => {
        allTableBody.appendChild(createRow(monster));
    });

    // Update admin kills table
    adminKills.forEach(adminKill => {
        adminKillsTableBody.appendChild(createRow(adminKill, false, false, true));
    });
}

setInterval(fetchMonsterData, 5000);

// Fetch data initially
window.onload = fetchMonsterData;

// Add event listeners to buttons
document.getElementById('apply-filters').addEventListener('click', applyFilters);
document.getElementById('reset-filters').addEventListener('click', resetFilters);

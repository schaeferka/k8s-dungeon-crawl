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
        const activeMonstersResponse = await fetch('/monsters/active');
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

    // Filter active monsters
    const filteredActiveMonsters = activeMonstersData.filter(monster => {
        if (!monster || !monster.type) return false;
        const matchesType = monster.type.toLowerCase().includes(typeFilter);
        const matchesDepth = depthFilter ? monster.depth == parseInt(depthFilter) : true;
        return matchesType && matchesDepth;
    });

    // Filter dead monsters
    const filteredDeadMonsters = deadMonstersData.filter(monster => {
        if (!monster || !monster.type) return false;
        const matchesType = monster.type.toLowerCase().includes(typeFilter);
        const matchesDepth = depthFilter ? monster.depth == parseInt(depthFilter) : true;
        return matchesType && matchesDepth;
    });

    // Filter all monsters
    const filteredAllMonsters = allMonstersData.filter(monster => {
        if (!monster || !monster.type) return false;
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
        row.appendChild(createTableCell(monster.is_admin_kill ? 'Yes' : 'No'));
        deadTableBody.appendChild(row);
    });

    // Update all monsters table (similar to the above method)
    allMonsters.forEach(monster => {
        allTableBody.appendChild(createMonsterRow(monster, monster.is_dead, false));
    });
}

setInterval(fetchMonsterData, 1000);

// Fetch data initially
window.onload = () => {
    fetchMonsterData();
};

// Add event listeners to buttons
document.getElementById('apply-filters').addEventListener('click', applyFilters);
document.getElementById('reset-filters').addEventListener('click', resetFilters);
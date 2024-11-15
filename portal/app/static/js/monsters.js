let currentMonstersData = [];
let deadMonstersData = [];
let overallMonstersData = [];

async function fetchMonsterData() {
    try {
        // Fetch current monsters data
        const currentGameResponse = await fetch('/monsters/data');
        currentMonstersData = await currentGameResponse.json();

        // Fetch dead monsters data
        const deadMonstersResponse = await fetch('/monsters/dead');
        deadMonstersData = await deadMonstersResponse.json();

        // Fetch overall monsters data
        const overallMonstersResponse = await fetch('/monsters/overall');
        overallMonstersData = await overallMonstersResponse.json();

        // Apply any filters that have been set
        applyFilters();
    } catch (error) {
        console.error('Error fetching monster data:', error);
    }
}

function applyFilters() {
    const typeFilter = document.getElementById('monster-type-filter').value.toLowerCase();
    const depthFilter = document.getElementById('depth-filter').value;

    // Filter data based on the inputs
    const filteredCurrentMonsters = currentMonstersData.filter(monster => {
        const matchesType = monster.type.toLowerCase().includes(typeFilter);
        const matchesDepth = depthFilter ? monster.depth == parseInt(depthFilter) : true;
        return matchesType && matchesDepth;
    });

    const filteredDeadMonsters = deadMonstersData.filter(monster => {
        const matchesType = monster.type.toLowerCase().includes(typeFilter);
        const matchesDepth = depthFilter ? monster.depth == parseInt(depthFilter) : true;
        return matchesType && matchesDepth;
    });

    const filteredOverallMonsters = overallMonstersData.filter(monster => {
        const matchesType = monster.type.toLowerCase().includes(typeFilter);
        const matchesDepth = depthFilter ? monster.depth == parseInt(depthFilter) : true;
        return matchesType && matchesDepth;
    });

    // Update tables with filtered data
    updateTables(filteredCurrentMonsters, filteredDeadMonsters, filteredOverallMonsters);
}

function resetFilters() {
    // Clear filter inputs and reset tables
    document.getElementById('monster-type-filter').value = '';
    document.getElementById('depth-filter').value = '';
    updateTables(currentMonstersData, deadMonstersData, overallMonstersData); // Reset tables with full data
}

function updateTables(currentMonsters, deadMonsters, overallMonsters) {
    const currentTableBody = document.getElementById('current-monsters-body');
    const deadTableBody = document.getElementById('dead-monsters-body');
    const overallTableBody = document.getElementById('overall-monsters-body');

    currentTableBody.innerHTML = '';
    deadTableBody.innerHTML = '';
    overallTableBody.innerHTML = '';

    // Update current monsters table
    currentMonsters.forEach(monster => {
        const row = document.createElement('tr');
        row.innerHTML = `
            <td class="text-center">${monster.name || ''}</td>
            <td class="text-center">${monster.hp || ''}</td>
            <td class="text-center">${monster.maxHP || ''}</td>
            <td class="text-center">${monster.depth || ''}</td>
            <td class="text-center">${monster.defense || ''}</td>
            <td class="text-center">${monster.accuracy || ''}</td>
            <td class="text-center">${monster.damageMin || ''} - ${monster.damageMax || ''}</td>
            <td class="text-center">${monster.turnsBetweenRegen || ''}</td>
            <td class="text-center">${monster.movementSpeed || ''}</td>
            <td class="text-center">${monster.attackSpeed || ''}</td>
            <td class="text-center">${monster.id || ''}</td>
            <td class="text-center">${monster.type || ''}</td>
        `;
        currentTableBody.appendChild(row);
    });

    // Update dead monsters table
    deadMonsters.forEach(monster => {
        const row = document.createElement('tr');
        row.innerHTML = `
            <td class="text-center">${monster.name || ''}</td>
            <td class="text-center">${monster.depth || ''}</td>
            <td class="text-center">${monster.hp || '0'} / ${monster.maxHP || ''}</td>
            <td class="text-center">${monster.id || ''}</td>
        `;
        deadTableBody.appendChild(row);
    });

    // Update overall monsters table
    overallMonsters.forEach(monster => {
        const row = document.createElement('tr');
        row.innerHTML = `
            <td class="text-center">${monster.name || ''}</td>
            <td class="text-center">${monster.hp || ''}</td>
            <td class="text-center">${monster.maxHP || ''}</td>
            <td class="text-center">${monster.depth || ''}</td>
            <td class="text-center">${monster.defense || ''}</td>
            <td class="text-center">${monster.accuracy || ''}</td>
            <td class="text-center">${monster.damageMin || ''} - ${monster.damageMax || ''}</td>
            <td class="text-center">${monster.movementSpeed || ''}</td>
            <td class="text-center">${monster.attackSpeed || ''}</td>
            <td class="text-center">${monster.id || ''}</td>
            <td class="text-center">${monster.type || ''}</td>
        `;
        overallTableBody.appendChild(row);
    });
}

setInterval(fetchMonsterData, 1000);

// Fetch data initially
window.onload = fetchMonsterData;

// Add event listeners to buttons
document.getElementById('apply-filters').addEventListener('click', applyFilters);
document.getElementById('reset-filters').addEventListener('click', resetFilters);


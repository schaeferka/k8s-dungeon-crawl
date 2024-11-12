async function fetchMonsterData() {
    try {
        const currentGameResponse = await fetch('/monsters/data');
        const currentGameData = await currentGameResponse.json();

        const deadMonstersResponse = await fetch('/dead_monsters');
        const deadMonstersData = await deadMonstersResponse.json();

        const overallMonstersResponse = await fetch('/overall_monsters');
        const overallMonstersData = await overallMonstersResponse.json();

        updateTables(
            currentGameData.current_game_monsters, 
            deadMonstersData.dead_monsters, 
            overallMonstersData.overall_monsters
        );
    } catch (error) {
        console.error('Error fetching monster data:', error);
    }
}

function updateTables(currentMonsters, deadMonsters, overallMonsters) {
    const currentTableBody = document.getElementById('current-monsters-body');
    const deadTableBody = document.getElementById('dead-monsters-body');
    const overallTableBody = document.getElementById('overall-monsters-body');

    currentTableBody.innerHTML = '';
    deadTableBody.innerHTML = '';
    overallTableBody.innerHTML = '';

    currentMonsters.forEach(monster => {
        const row = document.createElement('tr');
        row.innerHTML = `
            <td class="text-center">${monster.name || ''}</td>
            <td class="text-center">${monster.hp || ''}</td>
            <td class="text-center">${monster.maxHP || ''}</td>
            <td class="text-center">${monster.level || ''}</td>
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

    deadMonsters.forEach(monster => {
        const row = document.createElement('tr');
        row.innerHTML = `
            <td class="text-center">${monster.name || ''}</td>
            <td class="text-center">${monster.level || ''}</td>
            <td class="text-center">${monster.hp || '0'} / ${monster.maxHP || ''}</td>
            <td class="text-center">${monster.id || ''}</td>
        `;
        deadTableBody.appendChild(row);
    });

    overallMonsters.forEach(monster => {
        const row = document.createElement('tr');
        row.innerHTML = `
            <td class="text-center">${monster.name || ''}</td>
            <td class="text-center">${monster.hp || ''}</td>
            <td class="text-center">${monster.maxHP || ''}</td>
            <td class="text-center">${monster.level || ''}</td>
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

// Fetch and update data every 1 second
setInterval(fetchMonsterData, 1000);
window.onload = fetchMonsterData;

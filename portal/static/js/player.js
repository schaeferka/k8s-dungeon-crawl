async function fetchPlayerData() {
    try {
        const [playerResponse, itemsResponse, packResponse] = await Promise.all([
            fetch('/player/data'),
            fetch('/items/data'),
            fetch('/pack/data')
        ]);
        
        const playerData = await playerResponse.json();
        const itemsData = await itemsResponse.json();
        const packData = await packResponse.json();

        const combinedData = { ...playerData, ...itemsData, pack: packData.pack || [] };
        
        updatePlayerInfo(combinedData);
    } catch (error) {
        console.error('Error fetching player data:', error);
    }
}

function updatePlayerInfo(playerInfo) {
    const playerStatsDiv = document.getElementById('player-stats');
    const itemsDiv = document.getElementById('items');
    const inventoryDiv = document.getElementById('inventory-items');
    const modifiersDiv = document.getElementById('modifiers');

    playerStatsDiv.innerHTML = '';
    itemsDiv.innerHTML = '';
    inventoryDiv.innerHTML = '';
    modifiersDiv.innerHTML = '';

    const playerStatsKeys = [
        "currentHP", "gold", "maxHP", "poisonAmount", "regenPerTurn",
        "stealthRange", "strength", "weaknessAmount"
    ];
    const itemKeys = ["weapon", "armor", "ringLeft", "ringRight"];
    const modifierKeys = [
        "awarenessBonus", "clairvoyance", "disturbed", "lightMultiplier",
        "reaping", "regenerationBonus", "stealthBonus", "transference", "wisdomBonus"
    ];

    // Populate Player Stats section
    playerStatsKeys.forEach(key => {
        const statValue = key in playerInfo ? playerInfo[key] : "N/A";
        const statItem = document.createElement('p');
        statItem.textContent = `${key}: ${statValue}`;
        playerStatsDiv.appendChild(statItem);
    });

    // Populate Items section with equipped items
    itemKeys.forEach(key => {
        const item = playerInfo[key];
        if (item && typeof item === 'object') {
            const itemSection = document.createElement('div');
            itemSection.classList.add('item-section');
            itemSection.innerHTML = `<strong>(${key}):</strong><br>
                                     <p>${item.description || "No description"}</p>`;

            for (const [itemKey, itemValue] of Object.entries(item)) {
                if (itemKey !== "name" && itemKey !== "description") {
                    const itemDetail = document.createElement('p');
                    itemDetail.className = 'item-detail';
                    itemDetail.textContent = `${itemKey}: ${itemValue}`;
                    itemSection.appendChild(itemDetail);
                }
            }
            itemsDiv.appendChild(itemSection);
        }
    });

    // Populate Inventory section with items from the pack
    if (playerInfo.pack && playerInfo.pack.length > 0) {
        playerInfo.pack.forEach(item => {
            const itemDiv = document.createElement('div');
            itemDiv.classList.add('item-section');
            itemDiv.innerHTML = `
                <strong>${item.name || "Unknown Item"}</strong> (${item.category || "Unknown Category"}):<br>
                Quantity: ${item.quantity || 1}<br>
                Armor: ${item.armor || 0}<br>
                Damage: ${item.damage ? `${item.damage.min || 0} - ${item.damage.max || 0}` : 'N/A'}<br>
                Description: ${item.description || "No description"}<br>
                Inventory Letter: ${item.inventoryLetter || "N/A"}<br><br>
            `;
            inventoryDiv.appendChild(itemDiv);
        });
    } else {
        inventoryDiv.innerHTML = '<p>No items in inventory.</p>';
    }

    // Populate Modifiers section
    modifierKeys.forEach(key => {
        const modifierValue = key in playerInfo ? playerInfo[key] : "N/A";
        const modifierItem = document.createElement('p');
        modifierItem.textContent = `${key}: ${modifierValue}`;
        modifiersDiv.appendChild(modifierItem);
    });
}

// Fetch and update data every second
setInterval(fetchPlayerData, 1000);
window.onload = fetchPlayerData;

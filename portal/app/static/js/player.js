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
        "current_hp", "gold", "max_hp", "poison_amount", "regen_per_turn",
        "stealth_range", "strength", "weakness_amount"
    ];
    const itemKeys = ["weapon", "armor", "left_ring", "right_ring"];
    const modifierKeys = [
        "awareness_bonus", "clairvoyance", "disturbed", "light_multiplier",
        "reaping", "regeneration_bonus", "stealth_bonus", "transference", "wisdom_bonus"
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
        const titleMap = {
            weapon: "Weapon",
            armor: "Armor",
            left_ring: "Left Ring",
            right_ring: "Right Ring"
        };
        const icons = {
            weapon: "⚔️",
            armor: "🛡️",
            left_ring: "💍",
            right_ring: "💍"
        };

        const itemSection = document.createElement('div');
        itemSection.classList.add('item-section');
        itemSection.classList.add('border', 'p-4', 'rounded', 'my-2', 'shadow-lg');

        // Check if the item is equipped (has an inventoryLetter)
        if (item && item.inventory_letter && item.category !== "Unknown") {
            // Set section title with icon
            itemSection.innerHTML = `
                <strong>${icons[key]} ${titleMap[key]}:</strong><br>
            `;

            // Add description if it's not "No description"
            if (item.description && item.description !== "No description") {
                itemSection.innerHTML += `<p>Description: ${item.description}</p>`;
            }

            // Add other properties for the equipped items (like name, damage, etc.)
            if (key === "weapon" && item.damage) {
                itemSection.innerHTML += `<p>Damage: ${item.damage.min || 0} - ${item.damage.max || 0}</p>`;
            }
            
            // Display other properties for all items (excluding name and description)
            for (const [itemKey, itemValue] of Object.entries(item)) {
                if (itemKey !== "name" && itemKey !== "description" && itemKey !== "damage") {
                    itemSection.innerHTML += `<p>${itemKey}: ${itemValue}</p>`;
                }
            }
        } else {
            // Display "Not Equipped" if the item is not equipped
            itemSection.innerHTML = `
                <strong>${icons[key]} ${titleMap[key]}:</strong><br>
                <p>Not Equipped</p>
            `;
        }

        // Append item section to the items div
        itemsDiv.appendChild(itemSection);
    });

    // Populate Inventory section with items from the pack
    if (playerInfo.pack && playerInfo.pack.length > 0) {
        playerInfo.pack.forEach(item => {
            const itemDiv = document.createElement('div');
            itemDiv.classList.add('item-section');
            itemDiv.classList.add('border', 'p-4', 'rounded', 'my-2', 'shadow-lg');
            itemDiv.innerHTML = `
                <strong>${item.kind || "Unknown Item"}</strong> (${item.category || "Unknown Category"}):<br>
                Quantity: ${item.quantity || 1}<br>
                Armor: ${item.armor || 0}<br>
                Damage: ${item.damage ? `${item.damage.min || 0} - ${item.damage.max || 0}` : 'N/A'}<br>
                Description: ${item.description || "No description"}<br>
                Inventory Letter: ${item.inventory_letter || "N/A"}<br><br>
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

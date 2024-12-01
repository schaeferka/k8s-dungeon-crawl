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

        const combinedData = { ...playerData, ...itemsData, pack:packData.pack || {} };
        
        updatePlayerInfo(combinedData);
    } catch (error) {
        console.error('Error fetching player data:', error);
    }
}

function createSectionElement(title, content) {
    const section = document.createElement('div');
    section.classList.add('item-section', 'my-4', 'p-4', 'border', 'rounded', 'bg-gray-50', 'shadow-sm');
    section.innerHTML = title ? `<span class="text-lg font-semibold text-gray-800">${title}</span>${content}` : content;
    return section;
}

function populateSection(keys, data, container) {
    container.innerHTML = '';
    keys.forEach(key => {
        const value = key in data ? data[key] : "N/A";
        const item = document.createElement('p');
        item.classList.add('text-gray-600');
        item.textContent = `${key}: ${value}`;
        container.appendChild(item);
    });
}

function updatePlayerInfo(playerInfo) {
    const playerStatsDiv = document.getElementById('player-stats');
    const itemsDiv = document.getElementById('items');
    const inventoryDiv = document.getElementById('inventory-items');
    const modifiersDiv = document.getElementById('modifiers');

    const playerStatsKeys = [
        "current_hp", "gold", "max_hp", "poison_amount", "regen_per_turn",
        "stealth_range", "strength", "weakness_amount"
    ];
    const itemKeys = ["weapon", "armor", "left_ring", "right_ring"];
    const modifierKeys = [
        "awareness_bonus", "clairvoyance", "disturbed", "light_multiplier",
        "reaping", "regeneration_bonus", "stealth_bonus", "transference", "wisdom_bonus"
    ];

    populateSection(playerStatsKeys, playerInfo, playerStatsDiv);

    itemsDiv.innerHTML = '';
    const icons = {
        weapon: '<img src="/static/images/sword.png" alt="Weapon" class="inline-block w-10 h-10 mr-2">',
        armor: '<img src="/static/images/armor.png" alt="Armor" class="inline-block w-10 h-10 mr-2">',
        left_ring: '<img src="/static/images/ring.png" alt="Left Ring" class="inline-block w-10 h-10 mr-2">',
        right_ring: '<img src="/static/images/ring.png" alt="Right Ring" class="inline-block w-10 h-10 mr-2">'
    };
    itemKeys.forEach(key => {
        const item = playerInfo[key];
        const titleMap = {
            weapon: "Weapon",
            armor: "Armor",
            left_ring: "Left Ring",
            right_ring: "Right Ring"
        };

        let content = item && item.inventory_letter && item.category !== "Unknown" ? `
            ${item.description && item.description !== "No description" ? `<p>Description: ${item.description}</p>` : ''}
            ${key === "weapon" && item.damage ? `<p>Damage: ${item.damage.min || 0} - ${item.damage.max || 0}</p>` : ''}
            ${Object.entries(item).filter(([k]) => !['name', 'description', 'damage'].includes(k)).map(([k, v]) => `<p>${k}: ${v}</p>`).join('')}
        ` : '<p>Not Equipped</p>';

        itemsDiv.appendChild(createSectionElement(`${icons[key]} ${titleMap[key]}`, content));
    });

    inventoryDiv.innerHTML = '';
    if (playerInfo.pack && Object.keys(playerInfo.pack).length > 0) {
        Object.values(playerInfo.pack).forEach(item => {
            const content = `
                <span class="text-lg font-semibold text-gray-800">${item.kind || "Unknown Item"} - (${item.category || "Unknown Category"})</span><br>
                Quantity: ${item.quantity || 1}<br>
                Armor: ${item.armor || 0}<br>
                Damage: ${item.damage ? `${item.damage.min || 0} - ${item.damage.max || 0}` : 'N/A'}<br>
                Description: ${item.description || "No description"}<br>
                Inventory Letter: ${item.inventory_letter || "N/A"}
            `;
            inventoryDiv.appendChild(createSectionElement('', content));
        });
    } else {
        inventoryDiv.innerHTML = '<p>No items in inventory.</p>';
    }

    populateSection(modifierKeys, playerInfo, modifiersDiv);
}

// Fetch and update data every second
setInterval(fetchPlayerData, 1000);
window.onload = fetchPlayerData;

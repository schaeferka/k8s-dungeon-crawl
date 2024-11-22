async function fetchGameStateData() {
    try {
        const response = await fetch('/gamestate/data');
        const data = await response.json();
        updateGameStateInfo(data);
    } catch (error) {
        console.error('Error fetching game state data:', error);
    }
}

async function fetchGameStatsData() {
    try {
        const response = await fetch('/gamestats/data');
        const data = await response.json();
        updateGameStatsInfo(data);
    } catch (error) {
        console.error('Error fetching game stats data:', error);
    }
}

function updateGameStateInfo(gameState) {
    const gameStateDiv = document.getElementById('game-state-info');
    gameStateDiv.innerHTML = '';

    const sections = {
        "Basic Info": ["wizard", "gameInProgress", "gameHasEnded", "easyMode", "rewardRoomsGenerated"],
        "Game Timers and XP": ["absoluteTurnNumber", "milliseconds", "xpxpThisTurn", "monsterSpawnFuse"],
        "Gold and Resources": ["goldGenerated", "seed", "RNG"],
        "Game Depth": ["currentDepth", "deepestLevel"]
    };

    for (const [sectionTitle, keys] of Object.entries(sections)) {
        const sectionDiv = document.createElement('div');
        sectionDiv.classList.add('game-state-section', 'my-4', 'p-4', 'border', 'rounded', 'bg-gray-50', 'shadow-sm');

        const sectionHeader = document.createElement('h3');
        sectionHeader.classList.add('text-lg', 'font-semibold', 'text-gray-800');
        sectionHeader.textContent = sectionTitle;
        sectionDiv.appendChild(sectionHeader);

        keys.forEach(key => {
            const value = gameState[key] !== undefined ? gameState[key] : "N/A";
            const infoItem = document.createElement('p');
            infoItem.classList.add('text-gray-600');
            infoItem.textContent = `${key}: ${value}`;
            sectionDiv.appendChild(infoItem);
        });

        gameStateDiv.appendChild(sectionDiv);
    }
}

function updateGameStatsInfo(gameStats) {
    const gameStatsDiv = document.getElementById('game-stats-info');
    gameStatsDiv.innerHTML = '';

    const statsSections = [
        {
            title: "Gameplay Stats",
            keys: ["games", "escaped", "mastered", "won", "winRate", "currentWinStreak"]
        },
        {
            title: "Level Stats",
            keys: ["deepestLevel", "cumulativeLevels", "highestScore", "cumulativeScore"]
        },
        {
            title: "Gold and Lumenstone Stats",
            keys: ["mostGold", "cumulativeGold", "mostLumenstones", "cumulativeLumenstones"]
        },
        {
            title: "Turn and Streak Stats",
            keys: ["fewestTurnsWin", "cumulativeTurns", "longestWinStreak", "longestMasteryStreak", "currentMasteryStreak"]
        }
    ];

    statsSections.forEach(section => {
        const sectionDiv = document.createElement('div');
        sectionDiv.classList.add('game-stats-section', 'my-4', 'p-4', 'border', 'rounded', 'bg-gray-50', 'shadow-sm');

        const sectionHeader = document.createElement('h3');
        sectionHeader.classList.add('text-lg', 'font-semibold', 'text-gray-800');
        sectionHeader.textContent = section.title;
        sectionDiv.appendChild(sectionHeader);

        section.keys.forEach(key => {
            const value = gameStats[key] !== undefined ? gameStats[key] : "N/A";
            const infoItem = document.createElement('p');
            infoItem.classList.add('text-gray-600');
            infoItem.textContent = `${key}: ${value}`;
            sectionDiv.appendChild(infoItem);
        });

        gameStatsDiv.appendChild(sectionDiv);
    });
}

setInterval(fetchGameStateData, 1000);
setInterval(fetchGameStatsData, 1000);

window.onload = () => {
    fetchGameStateData();
    fetchGameStatsData();
};

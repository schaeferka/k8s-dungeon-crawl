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
        "Game Timers": ["absoluteTurnNumber", "milliseconds", "xpxpThisTurn", "monsterSpawnFuse"],
        "Gold and Resources": ["goldGenerated", "seed", "RNG"],
        "Game Depth": ["depthLevel", "deepestLevel"]
    };

    for (const [sectionTitle, keys] of Object.entries(sections)) {
        const sectionDiv = document.createElement('div');
        sectionDiv.classList.add('game-state-section');

        const sectionHeader = document.createElement('h3');
        sectionHeader.textContent = sectionTitle;
        sectionDiv.appendChild(sectionHeader);

        keys.forEach(key => {
            const value = gameState[key] !== undefined ? gameState[key] : "N/A";
            const infoItem = document.createElement('p');
            infoItem.textContent = `${key}: ${value}`;
            sectionDiv.appendChild(infoItem);
        });

        gameStateDiv.appendChild(sectionDiv);
    }
}

function updateGameStatsInfo(gameStats) {
    const gameStatsDiv = document.getElementById('game-stats-info');
    gameStatsDiv.innerHTML = '';

    const statsKeys = [
        "games", "escaped", "mastered", "won", "winRate", "deepestLevel",
        "cumulativeLevels", "highestScore", "cumulativeScore", "mostGold",
        "cumulativeGold", "mostLumenstones", "cumulativeLumenstones",
        "fewestTurnsWin", "cumulativeTurns", "longestWinStreak",
        "longestMasteryStreak", "currentWinStreak", "currentMasteryStreak"
    ];

    statsKeys.forEach(key => {
        const value = gameStats[key] !== undefined ? gameStats[key] : "N/A";
        const infoItem = document.createElement('p');
        infoItem.textContent = `${key}: ${value}`;
        gameStatsDiv.appendChild(infoItem);
    });
}

setInterval(fetchGameStateData, 1000);
setInterval(fetchGameStatsData, 1000);

window.onload = () => {
    fetchGameStateData();
    fetchGameStatsData();
};

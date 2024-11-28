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
        "Basic Info": ["wizard", "game_in_progress", "game_has_ended", "easy_mode", "reward_rooms_generated"],
        "Game Timers and XP": ["absolute_turn_number", "milliseconds", "xpxp_this_turn", "monster_spawn_fuse"],
        "Gold and Resources": ["gold_generated", "seed", "rng"],
        "Game Depth": ["current_depth", "deepest_level"]
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
            keys: ["games", "escaped", "mastered", "won", "win_rate", "current_win_streak"]
        },
        {
            title: "Level Stats",
            keys: ["deepest_level", "cumulative_levels", "highest_score", "cumulative_score"]
        },
        {
            title: "Gold and Lumenstone Stats",
            keys: ["most_gold", "cumulative_gold", "most_lumenstones", "cumulative_lumenstones"]
        },
        {
            title: "Turn and Streak Stats",
            keys: ["fewest_turns_win", "cumulative_turns", "longest_win_streak", "longest_mastery_streak", "current_mastery_streak"]
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
            const value = gameStats[key] !== undefined ? gameStats[key] : "0";
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

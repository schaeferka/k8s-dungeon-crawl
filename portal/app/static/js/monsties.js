async function fetchDeploymentsAndPods() {
    try {
        const response = await fetch('/monsties/deployments-pods');
        const data = await response.json();
        updateDeploymentsPodsTable(data);
    } catch (error) {
        console.error('Error fetching deployments and pods:', error);
    }
}

async function fetchMonsties() {
    try {
        const response = await fetch('/monsties/data');
        const data = await response.json();
        updateMonstiesTable(data);
    } catch (error) {
        console.error('Error fetching monstie data:', error);
    }
}

function updateDeploymentsPodsTable(data) {
    console.log('Fetched deployments and pods:', data);  // Debugging log
    const tableBody = document.getElementById('deployments-pods-body');
    tableBody.innerHTML = '';

    Object.entries(data).forEach(([deploymentName, deployment]) => {
        console.log(`Deployment: ${deploymentName}, Pods:`, deployment.pods);  // Debugging log
        
        // Create a row for the deployment with light gray shading
        const deploymentRow = document.createElement('tr');
        deploymentRow.classList.add('bg-gray-200');
        deploymentRow.innerHTML = `
            <td class="border px-4 py-2 font-bold text-center">${deploymentName}</td>
            <td class="border px-4 py-2 font-bold text-center">${deployment.replicas}</td>
            <td class="border px-4 py-2"></td>
            <td class="border px-4 py-2"></td>
            <td class="border px-4 py-2 text-center">
                <button class="delete-deployment px-4 py-2 bg-red-600 text-white rounded hover:bg-red-700 transition active:bg-red-900" data-deployment="${deploymentName}">Delete Deployment</button>
            </td>
        `;
        tableBody.appendChild(deploymentRow);
        
        // Create rows for each pod
        deployment.pods.forEach(pod => {
            const podRow = document.createElement('tr');
            podRow.innerHTML = `
                <td class="border px-4 py-2"></td>
                <td class="border px-4 py-2"></td>
                <td class="border px-4 py-2 text-center">${pod.name}</td>
                <td class="border px-4 py-2 text-center">${pod.status}</td>
                <td class="border px-4 py-2 text-center">
                    <button class="delete-pod px-4 py-2 bg-red-600 text-white rounded hover:bg-red-700 transition active:bg-red-900" data-pod="${pod.name}">Delete Pod</button>
                </td>
            `;
            tableBody.appendChild(podRow);
        });
    });

    // Add event listeners for delete buttons
    document.querySelectorAll('.delete-deployment').forEach(button => {
        button.addEventListener('click', async (event) => {
            const deploymentName = event.target.dataset.deployment;
            await fetch(`/monsties/delete-deployment/${deploymentName}`, { method: 'DELETE' });
            fetchDeploymentsAndPods();
        });
    });

    document.querySelectorAll('.delete-pod').forEach(button => {
        button.addEventListener('click', async (event) => {
            const podName = event.target.dataset.pod;
            await fetch(`/monsties/delete-pod/${podName}`, { method: 'DELETE' });
            fetchDeploymentsAndPods();
        });
    });
}

function updateMonstiesTable(data) {
    const tableBody = document.getElementById("monsties-body");
    tableBody.innerHTML = "";

    data.forEach(monster => {
        const row = document.createElement("tr");
        row.innerHTML = `
            <td class="border px-4 py-2 text-center">${monster.name}</td>
            <td class="border px-4 py-2 text-center">${monster.pod_name}</td>
            <td class="border px-4 py-2 text-center">${monster.hp}</td>
            <td class="border px-4 py-2 text-center">${monster.id}</td>
            <td class="border px-4 py-2 text-center">
                <button class="delete-monster px-4 py-2 bg-red-600 text-white rounded hover:bg-red-700 transition active:bg-red-900"
                    data-monster-name="${monster.pod_name}">
                    Admin Kill Monstie
                </button>
            </td>
        `;
        tableBody.appendChild(row);
    });

    document.addEventListener("click", async (event) => {
        if (event.target.classList.contains("delete-monster")) {
            const monsterName = event.target.dataset.monsterName;
            if (!monsterName) return;
            try {
                await fetch(`/monsters/admin-kill/pod/${monsterName}`, { method: "DELETE" });
                fetchMonsties(); // Refresh monster list
            } catch (error) {
                console.error("Error performing admin kill:", error);
            }
        }
    });
}


/* document.getElementById('admin-kill').addEventListener('click', async () => {
    const monsterName = prompt('Enter the monster pod name to kill:');
    if (!monsterName) return;
    try {
        await fetch(`/monsters/admin-kill/pod/${monsterName}`, { method: 'DELETE' });
        fetchDeploymentsAndPods();
    } catch (error) {
        console.error('Error performing admin kill:', error);
    }
}); */

document.getElementById('create-deployment').addEventListener('click', async () => {
    try {
        await fetch('/monsties/create-deployment', { method: 'POST' });
        fetchDeploymentsAndPods();
    } catch (error) {
        console.error('Error creating deployment:', error);
    }
});

setInterval(fetchDeploymentsAndPods, 1000);
setInterval(fetchMonsties, 1000);

window.onload = () => {
    fetchDeploymentsAndPods();
    fetchMonsties();
};

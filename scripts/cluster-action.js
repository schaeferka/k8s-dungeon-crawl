require('dotenv').config();
const { exec } = require('child_process');
const logger = require('./logger');

// Get the cluster name and action argument from the command line
const clusterName = process.env.KDC_CLUSTER_NAME;
const actionArg = process.argv[2];  // Action to perform: create, delete, start, stop

if (!clusterName) {
    logger.error('Error: KDC_CLUSTER_NAME is not defined in the .env file.');
    process.exit(1);
}

if (!actionArg) {
    logger.error('Error: No action specified. Usage: create, delete, start, stop');
    process.exit(1);
}

async function createCluster() {
    logger.info(`Cluster '${clusterName}' is being created`);
    const command = `k3d cluster create ${clusterName} --port "$KDC_LOCAL_PORT_8090:16080@loadbalancer" --port "$KDC_LOCAL_PORT_8010:38010@loadbalancer" --port "$KDC_LOCAL_PORT_5910:35910@loadbalancer" --port "$KDC_LOCAL_PORT_5000:35000@loadbalancer"
`;
    exec(command, (error, stdout, stderr) => {
        if (error) {
            logger.error(`Error executing command: ${error.message}`);
            process.exit(1);
        }
        if (stderr) {
            logger.error(`stderr: ${stderr}`);
            process.exit(1);
        }

        logger.info(`Cluster '${clusterName}' created successfully`);
        checkClusterReady(clusterName); 
    });
}

async function deleteCluster() {
    logger.info(`Cluster '${clusterName}' is being deleted`);
    const command = `k3d cluster delete ${clusterName}`;

    exec(command, (error, stdout, stderr) => {
        if (error) {
            logger.error(`Error executing command: ${error.message}`);
            process.exit(1);
        }
        if (stderr) {
            logger.error(`stderr: ${stderr}`);
            process.exit(1);
        }

        logger.info(`Cluster '${clusterName}' deleted successfully`);
    });
}

async function startCluster() {
    logger.info(`Starting cluster '${clusterName}'`);
    const command = `k3d cluster start ${clusterName}`;

    exec(command, (error, stdout, stderr) => {
        if (error) {
            logger.error(`Error executing command: ${error.message}`);
            process.exit(1);
        }
        if (stderr) {
            logger.error(`stderr: ${stderr}`);
            process.exit(1);
        }

        logger.info(`Cluster '${clusterName}' started successfully`);
        checkClusterReady(clusterName);  
    });
}

async function stopCluster() {
    logger.info(`Stopping cluster '${clusterName}'`);
    const command = `k3d cluster stop ${clusterName}`;

    exec(command, (error, stdout, stderr) => {
        if (error) {
            logger.error(`Error executing command: ${error.message}`);
            process.exit(1);
        }
        if (stderr) {
            logger.error(`stderr: ${stderr}`);
            process.exit(1);
        }

        logger.info(`Cluster '${clusterName}' stopped successfully`);
    });
}

async function checkClusterReady(clusterName) {
    let retries = 10;
    let ready = false;

    // Helper function to check if kubectl is able to communicate with the cluster
    const checkAPI = () => {
        return new Promise((resolve, reject) => {
            exec('kubectl cluster-info', (error, stdout, stderr) => {
                if (error) {
                    reject(new Error(`Error communicating with cluster: ${error.message}`));
                }
                if (stderr) {
                    reject(new Error(`stderr: ${stderr}`));
                }
                resolve(stdout);
            });
        });
    };

    // Helper function to check if CoreDNS pods are running
    const checkCoreDNS = () => {
        return new Promise((resolve, reject) => {
            exec('kubectl get pods -n kube-system -l k8s-app=kube-dns -o jsonpath="{.items[0].status.phase}"', (error, stdout, stderr) => {
                if (error || stderr) {
                    reject(new Error(`Error checking CoreDNS: ${stderr || error.message}`));
                }
                if (stdout === 'Running') {
                    resolve(true);
                } else {
                    reject(new Error('CoreDNS not ready'));
                }
            });
        });
    };

    // Helper function to check if Metrics Server pod is running
    const checkMetricsServer = () => {
        return new Promise((resolve, reject) => {
            exec('kubectl get pods -n kube-system -l k8s-app=metrics-server -o jsonpath="{.items[0].status.phase}"', (error, stdout, stderr) => {
                if (error || stderr) {
                    reject(new Error(`Error checking Metrics Server: ${stderr || error.message}`));
                }
                if (stdout === 'Running') {
                    resolve(true);
                } else {
                    reject(new Error('Metrics Server not ready'));
                }
            });
        });
    };

    // Verify the readiness of the cluster
    while (retries > 0) {
        try {
            // First, check if the Kubernetes API server is responsive
            await checkAPI();

            // Wait for a bit to give the CoreDNS pods time to start
            logger.info('Waiting for CoreDNS to start...');
            await new Promise(resolve => setTimeout(resolve, 30 * 1000)); // Wait 30 seconds

            // Now check if CoreDNS and Metrics Server are running
            await checkCoreDNS();
            await checkMetricsServer();

            ready = true;
            break;  // Exit if the cluster is ready
        } catch (error) {
            // Log error and retry
            logger.error(`Cluster '${clusterName}' not ready: ${error.message}`);
            retries--;
            if (retries > 0) {
                logger.info(`Retrying cluster readiness check... (${retries} retries left)`);
                await new Promise(resolve => setTimeout(resolve, 5000));  // Wait 5 seconds before retrying
            }
        }
    }

    // If the cluster didn't become ready after all retries, log and exit
    if (!ready) {
        logger.error(`Cluster '${clusterName}' failed to become ready after ${retries} attempts.`);
        process.exit(1);
    } else {
        logger.info(`Cluster '${clusterName}' is ready`);
    }
}

async function handleClusterAction(action) {
    switch (action) {
        case 'create':
            await createCluster();
            break;
        case 'delete':
            await deleteCluster();
            break;
        case 'start':
            await startCluster();
            break;
        case 'stop':
            await stopCluster();
            break;
        default:
            logger.error(`Invalid action '${action}'. Use 'create', 'delete', 'start', or 'stop'.`);
            process.exit(1);
    }
}

async function execute() {
    await handleClusterAction(actionArg);
}

execute();

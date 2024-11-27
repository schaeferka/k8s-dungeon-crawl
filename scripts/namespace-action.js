const k8s = require('@kubernetes/client-node');
const logger = require('./logger');
const fs = require('fs');

// Get the namespace and action arguments from the command line
const namespaceArg = process.argv[2];  // Namespace to be managed
const actionArg = process.argv[3];  // Action to perform: create, delete, or reset

if (!namespaceArg || !actionArg) {
    logger.error('Both namespace and action arguments are required. Usage: <namespace> <action>');
    process.exit(1);
}

// Initialize Kubernetes client
const kc = new k8s.KubeConfig();
kc.loadFromDefault();  // Loads kubeconfig from the default location (~/.kube/config)

// Create Kubernetes API client
const k8sApi = kc.makeApiClient(k8s.CoreV1Api);

// Function to check if a namespace exists
async function namespaceExists(namespace) {
    try {
        const res = await k8sApi.readNamespace(namespace);
        return res.body ? true : false;
    } catch (error) {
        if (error.response && error.response.statusCode === 404) {
            // If the namespace doesn't exist, return false
            return false;
        }
        // If there's any other error, log it and throw
        logger.error(`Error checking namespace '${namespace}': ${error.message}`);
        throw error;
    }
}

// Function to delete a namespace (only if it exists)
async function deleteNamespace(namespace) {
    const exists = await namespaceExists(namespace);
    if (exists) {
        try {
            await k8sApi.deleteNamespace(namespace);
            logger.info(`Namespace '${namespace}' deletion initiated.`);
            // Wait for the namespace to be fully deleted
            await waitForNamespaceDeletion(namespace);
            logger.info(`Namespace '${namespace}' deleted successfully.`);
        } catch (error) {
            logger.error(`Error deleting namespace '${namespace}': ${error.message}`);
            throw error;
        }
    } else {
        logger.info(`Namespace '${namespace}' does not exist. Skipping deletion.`);
    }
}

// Function to create a namespace
async function createNamespace(namespace) {
    const body = {
        apiVersion: 'v1',
        kind: 'Namespace',
        metadata: {
            name: namespace,
        },
    };

    try {
        // First, check if the namespace already exists
        const res = await k8sApi.readNamespace(namespace);
        if (res.body) {
            // If the namespace exists, log a short message and skip creation
            logger.info(`Namespace '${namespace}' already exists. Skipping creation.`);
            return;
        }
    } catch (error) {
        if (error.response && error.response.statusCode === 404) {
            // If the namespace does not exist, we proceed with creation
            logger.info(`Namespace '${namespace}' does not exist. Creating...`);
        } else {
            // If there is another error, log it and stop
            logger.error(`Error checking namespace '${namespace}': ${error.message}`);
            throw error;
        }
    }

    // Create the namespace since it doesn't exist
    try {
        const res = await k8sApi.createNamespace(body);
        logger.info(`Namespace '${namespace}' creation initiated.`);
        // Wait for the namespace to be fully created and ready
        await waitForNamespaceReady(namespace);
        logger.info(`Namespace '${namespace}' created successfully.`);
        return res.body;
    } catch (error) {
        logger.error(`Error creating namespace '${namespace}': ${error.message}`);
        throw error;
    }
}

// Function to reset a namespace (delete and recreate)
async function resetNamespace(namespace) {
    await deleteNamespace(namespace);  // Delete the namespace
    await createNamespace(namespace);  // Recreate the namespace
}

// Wait for a namespace to be fully created and ready
async function waitForNamespaceReady(namespace) {
    let retries = 10;
    while (retries > 0) {
        try {
            const res = await k8sApi.readNamespace(namespace);
            if (res.body.status.phase === 'Active') {
                logger.info(`Namespace '${namespace}' is ready.`);
                return;
            }
        } catch (error) {
            if (error.response && error.response.statusCode === 404) {
                logger.info(`Namespace '${namespace}' not found yet. Retrying...`);
            } else {
                logger.error(`Error checking status of namespace '${namespace}': ${error.message}`);
                throw error;
            }
        }
        retries--;
        if (retries > 0) {
            await new Promise(resolve => setTimeout(resolve, 2000)); // Wait 2 seconds before retrying
        }
    }
    logger.error(`Namespace '${namespace}' failed to become ready after retries.`);
    throw new Error(`Namespace '${namespace}' creation failed to complete in time.`);
}

// Wait for a namespace to be deleted
async function waitForNamespaceDeletion(namespace) {
    let retries = 10;
    while (retries > 0) {
        const exists = await namespaceExists(namespace);
        if (!exists) {
            logger.info(`Namespace '${namespace}' has been deleted.`);
            return;
        }
        retries--;
        if (retries > 0) {
            await new Promise(resolve => setTimeout(resolve, 2000)); // Wait 2 seconds before retrying
        }
    }
    logger.error(`Namespace '${namespace}' failed to delete after retries.`);
    throw new Error(`Namespace '${namespace}' deletion failed to complete in time.`);
}

// Function to handle namespace operations based on the action argument
async function handleNamespaceAction(namespace, action) {
    switch (action) {
        case 'create':
            await createNamespace(namespace);
            break;
        case 'delete':
            await deleteNamespace(namespace);
            break;
        case 'reset':
            await resetNamespace(namespace);
            break;
        default:
            logger.error(`Invalid action '${action}'. Use 'create', 'delete', or 'reset'.`);
            process.exit(1);
    }
}

// Run the script
async function execute() {
    await handleNamespaceAction(namespaceArg, actionArg);
}

// Start the script
execute();

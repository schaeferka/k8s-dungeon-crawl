const k8s = require('@kubernetes/client-node');
const logger = require('./logger');
const fs = require('fs');

const namespaceArg = process.argv[2];  // Namespace to be managed
const actionArg = process.argv[3];  // Action to perform: create, delete, or reset

if (!namespaceArg || !actionArg) {
    logger.error('Both namespace and action arguments are required. Usage: <namespace> <action>');
    process.exit(1);
}

const kc = new k8s.KubeConfig();
kc.loadFromDefault(); 

const k8sApi = kc.makeApiClient(k8s.CoreV1Api);

async function namespaceExists(namespace) {
    try {
        const res = await k8sApi.readNamespace(namespace);
        return res.body ? true : false;
    } catch (error) {
        if (error.response && error.response.statusCode === 404) {
            return false;
        }
        logger.error(`Error checking namespace '${namespace}': ${error.message}`);
        throw error;
    }
}

async function deleteNamespace(namespace) {
    const exists = await namespaceExists(namespace);
    if (exists) {
        try {
            await k8sApi.deleteNamespace(namespace);
            logger.info(`Namespace '${namespace}' deletion initiated.`);

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

async function createNamespace(namespace) {
    const body = {
        apiVersion: 'v1',
        kind: 'Namespace',
        metadata: {
            name: namespace,
        },
    };

    try {
        const res = await k8sApi.readNamespace(namespace);
        if (res.body) {
            logger.info(`Namespace '${namespace}' already exists. Skipping creation.`);
            return;
        }
    } catch (error) {
        if (error.response && error.response.statusCode === 404) {
            logger.info(`Namespace '${namespace}' does not exist. Creating...`);
        } else {
            logger.error(`Error checking namespace '${namespace}': ${error.message}`);
            throw error;
        }
    }

    try {
        const res = await k8sApi.createNamespace(body);
        logger.info(`Namespace '${namespace}' creation initiated.`);
        await waitForNamespaceReady(namespace);
        logger.info(`Namespace '${namespace}' created successfully.`);
        return res.body;
    } catch (error) {
        logger.error(`Error creating namespace '${namespace}': ${error.message}`);
        throw error;
    }
}

async function resetNamespace(namespace) {
    await deleteNamespace(namespace);  
    await createNamespace(namespace);  
}

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
            await new Promise(resolve => setTimeout(resolve, 2000));
        }
    }
    logger.error(`Namespace '${namespace}' failed to become ready after retries.`);
    throw new Error(`Namespace '${namespace}' creation failed to complete in time.`);
}

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
            await new Promise(resolve => setTimeout(resolve, 2000));
        }
    }
    logger.error(`Namespace '${namespace}' failed to delete after retries.`);
    throw new Error(`Namespace '${namespace}' deletion failed to complete in time.`);
}

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

async function execute() {
    await handleNamespaceAction(namespaceArg, actionArg);
}

execute();

require('dotenv').config();  
const { exec } = require('child_process');
const path = require('path');
const fs = require('fs');

const logger = require('./logger');
const envArg = process.argv[2];  // game, portal, controller, custom404
const actionArg = process.argv[3];  // Action to perform: build or import

if (!envArg || !actionArg) {
    logger.error('Both environment and action arguments are required. Usage: <game|portal|controller> <build|import>');
    process.exit(1);
}

let IMAGE_NAME, BUILD_DIR, NAMESPACE;

switch (envArg) {
    case 'game':
        IMAGE_NAME = process.env.KDC_GAME_IMAGE || 'game:latest';
        BUILD_DIR = process.env.KDC_GAME_DIR || './game';
        NAMESPACE = process.env.KDC_GAME_NS || 'game';
        break;
    case 'portal':
        IMAGE_NAME = process.env.KDC_PORTAL_IMAGE || 'portal:latest';
        BUILD_DIR = process.env.KDC_PORTAL_DIR || './portal';
        NAMESPACE = process.env.KDC_PORTAL_NS || 'portal';
        break;
    case 'controller':
        IMAGE_NAME = process.env.KDC_CONTROLLER_IMAGE || 'controller:latest';
        BUILD_DIR = process.env.KDC_CONTROLLER_DIR || './controller';
        NAMESPACE = process.env.KDC_CONTROLLER_NS || 'dungeon-master-service';
        break;
    case 'custom404':
        IMAGE_NAME = process.env.KDC_CUSTOM404_IMAGE || 'custom404:latest';
        BUILD_DIR = process.env.KDC_CUSTOM404_DIR || './custom404';
        NAMESPACE = process.env.KDC_CUSTOM404_NS || 'monsters';
        break;
    default:
        logger.error(`Invalid environment argument "${envArg}". Please specify "game", "portal", or "controller".`);
        process.exit(1);
}

if (actionArg === 'build') {
    const dockerfilePath = path.join(BUILD_DIR, 'Dockerfile');
    if (!fs.existsSync(dockerfilePath)) {
        logger.error(`Dockerfile not found in ${BUILD_DIR}`);
        process.exit(1);
    }
}

async function handleImageAction() {
    switch (actionArg) {
        case 'build':
            await buildImage();
            break;
        case 'import':
            await importImage();
            break;
        default:
            logger.error(`Invalid action '${actionArg}'. Use 'build' or 'import'.`);
            process.exit(1);
    }
}

async function buildImage() {
    logger.info(`Starting Docker image build for ${IMAGE_NAME} from directory ${BUILD_DIR}...`);
    const buildCommand = `docker build -t ${IMAGE_NAME} ${BUILD_DIR}`;

    const command = exec(buildCommand, (error, stdout, stderr) => {
        if (error) {
            logger.error(`Error executing build command: ${error.message}`);
            process.exit(1);
        }

        if (stderr && !stderr.includes('CACHED')) {
            // Only log actual stderr errors (ignores "CACHED" lines)
            logger.error(`stderr: ${stderr}`);
            process.exit(1);
        }
        logger.info(`Docker image '${IMAGE_NAME}' built successfully!`);
    });
}

async function importImage() {
    logger.info(`Starting to import Docker image '${IMAGE_NAME}' into k3d cluster '${process.env.KDC_CLUSTER_NAME}'...`);

    const clusterName = process.env.KDC_CLUSTER_NAME;

    if (!clusterName) {
        logger.error('Error: KDC_CLUSTER_NAME is not defined in the .env file.');
        process.exit(1);
    }

    // Construct the k3d image import command
    const importCommand = `k3d image import ${IMAGE_NAME} -c ${clusterName}`;

    const command = exec(importCommand, (error, stdout, stderr) => {
        if (error) {
            logger.error(`Error executing import command: ${error.message}`);
            process.exit(1);
        }

        if (stderr) {
            logger.error(`stderr: ${stderr}`);
            process.exit(1);
        }
        logger.info(`Docker image '${IMAGE_NAME}' imported successfully into the k3d cluster '${clusterName}'!`);
    });
}

handleImageAction();

require('dotenv').config();  // Load environment variables from .env file
const { exec } = require('child_process');
const path = require('path');
const fs = require('fs');

// Import the logger from the logger.js file
const logger = require('./logger');

// Get the environment and action arguments from the command line
const envArg = process.argv[2];  // This will get the 3rd argument passed (game, portal, controller)
const actionArg = process.argv[3];  // Action to perform: build or import

if (!envArg || !actionArg) {
    logger.error('Both environment and action arguments are required. Usage: <game|portal|controller> <build|import>');
    process.exit(1);
}

// Define environment-specific variables based on the argument
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
    default:
        logger.error(`Invalid environment argument "${envArg}". Please specify "game", "portal", or "controller".`);
        process.exit(1);
}

// Check if the Dockerfile exists in the build directory (only for build action)
if (actionArg === 'build') {
    const dockerfilePath = path.join(BUILD_DIR, 'Dockerfile');
    if (!fs.existsSync(dockerfilePath)) {
        logger.error(`Dockerfile not found in ${BUILD_DIR}`);
        process.exit(1);
    }
}

// Determine what action to take based on the argument
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

// Build the Docker image
async function buildImage() {
    logger.info(`Starting Docker image build for ${IMAGE_NAME} from directory ${BUILD_DIR}...`);
    const buildCommand = `docker build -t ${IMAGE_NAME} ${BUILD_DIR}`;
    const logFile = path.join(__dirname, 'build_output.log');  // Specify the log file path

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

        // Output the build result to the console
        logger.info(stdout);

        // Check if the build was successful
        logger.info(`Docker image '${IMAGE_NAME}' built successfully!`);
    });

    // Redirect stdout and stderr to the log file
    command.stdout.pipe(fs.createWriteStream(logFile, { flags: 'a' }));
    command.stderr.pipe(fs.createWriteStream(logFile, { flags: 'a' }));
}

// Import the Docker image into Kubernetes
async function importImage() {
    logger.info(`Starting to import Docker image '${IMAGE_NAME}' into k3d cluster '${process.env.KDC_CLUSTER_NAME}'...`);

    const clusterName = process.env.KDC_CLUSTER_NAME; // Cluster name from environment variables

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

        // Output the import result to the console
        logger.info(stdout);

        // Check if the import was successful
        logger.info(`Docker image '${IMAGE_NAME}' imported successfully into the k3d cluster '${clusterName}'!`);
    });
}

// Run the script
handleImageAction();

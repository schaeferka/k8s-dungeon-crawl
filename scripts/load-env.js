const fs = require('fs');
const path = require('path');
const yargs = require('yargs');

// Import the logger from the logger.js file
const logger = require('./logger');

// Parse command line arguments using yargs
const argv = yargs
  .option('print', {
    alias: 'p',
    type: 'boolean',
    description: 'Print the environment variables to the console',
    default: false,
  })
  .argv;

const envFilePath = path.resolve(__dirname, '../.env');

// Check if the .env file exists
if (fs.existsSync(envFilePath)) {
  // Read the .env file and load the environment variables
  require('dotenv').config({ path: envFilePath });
  logger.info('Environment variables loaded from .env');
  
  // Optionally log KDC_ prefixed environment variables if --print flag is set
  if (argv.print) {
    const kdcEnvVars = Object.keys(process.env).filter(key => key.startsWith('KDC_'));
    if (kdcEnvVars.length > 0) {
      logger.info('KDC Environment Variables:');
      kdcEnvVars.forEach((key) => {
        logger.info(`${key}=${process.env[key]}`);
      });
    } else {
      logger.info('No KDC environment variables found.');
    }
  }
} else {
  logger.error('.env file not found!');
  process.exit(1); // Exit if .env file is not found
}

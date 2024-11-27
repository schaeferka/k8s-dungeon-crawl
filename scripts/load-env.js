const fs = require('fs');
const path = require('path');
const yargs = require('yargs');

const logger = require('./logger');

const argv = yargs
  .option('print', {
    alias: 'p',
    type: 'boolean',
    description: 'Print the environment variables to the console',
    default: false,
  })
  .argv;

const envFilePath = path.resolve(__dirname, '../.env');

if (fs.existsSync(envFilePath)) {
  require('dotenv').config({ path: envFilePath });
  logger.info('Environment variables loaded from .env');
  
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
  process.exit(1);
}

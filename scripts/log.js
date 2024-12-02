
const logger = require('./logger');

const [level, ...messageParts] = process.argv.slice(2);
const message = messageParts.join(' ');

switch (level) {
    case 'info':
        logger.info(message);
        break;
    case 'error':
        logger.error(message);
        break;
    case 'warn':
        logger.warn(message);
        break;
    default:
        logger.info(message);
        break;
}
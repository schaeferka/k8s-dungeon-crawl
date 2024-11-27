const pino = require('pino');
const pretty = require('pino-pretty');

// Suppress deprecation warnings globally
process.on('warning', (warning) => {
    if (warning.name === 'DeprecationWarning') {
        // Do nothing to suppress the DeprecationWarning
        return;
    }

    console.warn(warning.name, warning.message);
});

const logger = pino({
    level: process.env.LOG_LEVEL || 'info',
    transport: {
        target: 'pino-pretty',
        options: {
            translateTime: 'SYS:standard',
            colorize: true,                
            singleLine: true              
        }
    }
});

module.exports = logger;

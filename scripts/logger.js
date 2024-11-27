const pino = require('pino');
const pretty = require('pino-pretty');

// Suppress deprecation warnings globally
process.on('warning', (warning) => {
    if (warning.name === 'DeprecationWarning') {
        // Do nothing to suppress the DeprecationWarning
        return;
    }
    // Log other types of warnings (if needed)
    console.warn(warning.name, warning.message);
});

// Configure and create the logger
const logger = pino({
    level: process.env.LOG_LEVEL || 'info',
    transport: {
        target: 'pino-pretty',
        options: {
            translateTime: 'SYS:standard', // Format the timestamp
            colorize: true,                // Colorize the output
            singleLine: true               // Single line logs
        }
    }
});

module.exports = logger;

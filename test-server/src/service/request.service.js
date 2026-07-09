// Author:  Corey Ferguson
// Date:    2025 September 2
// File:    request.service.js
// Project: fetch-json
//

const { LoggerService } = require('./logger.service');

// Typdef

class RequestService {
    // Non-Member Functions
    
    static receive = (url, body) => LoggerService.info({
        date: new Date().toISOString().split('T').join(' '),
        url,
        body
    });
}

module.exports = { RequestService };
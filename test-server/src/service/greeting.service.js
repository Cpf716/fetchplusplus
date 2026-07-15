// Author:  Corey Ferguson
// Date:    2025 September 2
// File:    greeting.service.js
// Project: fetch-json
//
// Non-Member Fields
class GreetingService {
    /**
     * 
     * @param {{ firstName: string; lastName?: string | null, nickname?: string | null }} options 
     * @returns string
     */
    create(options) {
        if (!['firstName', 'nickname'].some(value => options[value])) {
            throw new Error("must have required property 'firstName'");
        }

        const greeting = ["Hello, ", options.firstName];

        options.lastName && greeting.push(' ', options.lastName);

        return [...greeting, '!'].join("");
    }
}

module.exports = { GreetingService }
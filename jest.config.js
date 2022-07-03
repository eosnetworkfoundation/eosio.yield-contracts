module.exports = {
    preset: 'ts-jest',
    testEnvironment: 'node',
    moduleNameMapper: {
        '^@tests/(.*)$': '<rootDir>/tests/$1',
    },
};
/** @type {import('ts-jest').JestConfigWithTsJest} */
module.exports = {
  preset: 'ts-jest',
  testEnvironment: 'node',
  roots: ['<rootDir>/src'],
  testMatch: ['**/*.test.ts'],
  moduleFileExtensions: ['ts', 'js', 'mjs'],
  // chevrotain v11+ is ESM-only; map to its self-contained .mjs bundle so
  // the custom transformer below can convert it to CJS without having to
  // resolve @chevrotain/* sub-packages that Jest cannot handle in CJS mode.
  moduleNameMapper: {
    '^vscode$': '<rootDir>/src/__mocks__/vscode.ts',
    '^chevrotain$': '<rootDir>/node_modules/chevrotain/lib/chevrotain.mjs',
  },
  // Do not ignore the chevrotain package — let the custom transformer handle it.
  transformIgnorePatterns: ['node_modules/(?!(chevrotain)/)'],
  transform: {
    '^.+\\.tsx?$': 'ts-jest',
    // Convert the self-contained chevrotain ESM bundle to CJS at test time.
    '^.+\\.mjs$': '<rootDir>/jest-esm-to-cjs.cjs',
  },
};

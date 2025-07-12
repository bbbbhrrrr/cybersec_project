const { execSync } = require('child_process');
const path = require('path');

/*
 * Test Runner Script
 *
 * This script runs the complete test suite for the Poseidon2 circuit.
 * It includes compilation, setup, and functional tests.
 */

async function main() {
 console.log('🧪 Running Poseidon2 Circuit Test Suite...\n');

 try {
 // Step 1: Compile circuit
 console.log('📦 Step 1: Compiling circuit...');
 execSync('node scripts/compile.js', {
 stdio: 'inherit',
 cwd: __dirname + '/..'
 });

 // Step 2: Setup trusted setup
 console.log('\n Step 2: Setting up trusted setup...');
 execSync('node scripts/setup.js', {
 stdio: 'inherit',
 cwd: __dirname + '/..'
 });

 // Step 3: Run unit tests
 console.log('\n🧪 Step 3: Running unit tests...');
 execSync('npx mocha tests/poseidon2.test.js --reporter spec', {
 stdio: 'inherit',
 cwd: __dirname + '/..'
 });

 // Step 4: Generate sample proof
 console.log('\n Step 4: Generating sample proof...');
 execSync('node scripts/prove.js', {
 stdio: 'inherit',
 cwd: __dirname + '/..'
 });

 // Step 5: Verify sample proof
 console.log('\n Step 5: Verifying sample proof...');
 execSync('node scripts/verify.js', {
 stdio: 'inherit',
 cwd: __dirname + '/..'
 });

 console.log('\n All tests passed successfully!');
 console.log('\n Test Summary:');
 console.log(' Circuit compilation');
 console.log(' Trusted setup');
 console.log(' Unit tests');
 console.log(' Proof generation');
 console.log(' Proof verification');

 } catch (error) {
 console.error('\n Test failed:');
 console.error(error.message);
 process.exit(1);
 }
}

if (require.main === module) {
 main().catch(console.error);
}

module.exports = { main };

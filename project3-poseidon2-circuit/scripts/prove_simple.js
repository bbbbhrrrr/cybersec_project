const fs = require('fs');
const path = require('path');
const snarkjs = require('snarkjs');

/*
 * Simple Proof Generation Test
 */

async function main() {
 console.log(' Testing Simple Poseidon2 Circuit...\n');

 const buildDir = path.join(__dirname, '../build');
 const setupDir = path.join(__dirname, '../setup');
 const proofsDir = path.join(__dirname, '../proofs');

 // Create proofs directory
 if (!fs.existsSync(proofsDir)) {
 fs.mkdirSync(proofsDir, { recursive: true });
 }

 // Use simple circuit for testing
 const wasmPath = path.join(buildDir, 'poseidon2_simple_js', 'poseidon2_simple.wasm');

 // Check if we have a simple setup, if not skip setup step
 const simpleZkeyPath = path.join(setupDir, 'poseidon2_simple_final.zkey');

 if (!fs.existsSync(simpleZkeyPath)) {
 console.error('�?Simple setup not found. Please run setup first.');
 process.exit(1);
 }

 try {
 console.log(' WASM file:', wasmPath);
 console.log(' Proving key:', simpleZkeyPath);
 console.log();

 // Simple inputs for the simplified circuit
 // The simple circuit computes: (preimage[0] + preimage[1])^5 + 12345
 const preimage = ["123", "456"];
 const expectedHash = Math.pow(123 + 456, 5) + 12345; // = 579^5 + 12345

 console.log(' Testing with simple inputs:');
 console.log(' Preimage[0]:', preimage[0]);
 console.log(' Preimage[1]:', preimage[1]);
 console.log(' Expected hash:', expectedHash);

 // Prepare circuit inputs
 const input = {
 "preimage": preimage,
 "hash": expectedHash.toString()
 };

 console.log('\n Generating witness...');
 const witness = await snarkjs.wtns.calculate(input, wasmPath);

 console.log(' Generating proof...');
 const startTime = Date.now();

 const { proof, publicSignals } = await snarkjs.groth16.prove(
 simpleZkeyPath,
 witness
 );

 const proofTime = Date.now() - startTime;

 // Save proof and public inputs
 const proofPath = path.join(proofsDir, 'proof_simple.json');
 const publicPath = path.join(proofsDir, 'public_simple.json');
 const inputPath = path.join(proofsDir, 'input_simple.json');

 fs.writeFileSync(proofPath, JSON.stringify(proof, null, 2));
 fs.writeFileSync(publicPath, JSON.stringify(publicSignals, null, 2));
 fs.writeFileSync(inputPath, JSON.stringify(input, null, 2));

 console.log('\n�?Proof generated successfully!');
 console.log('\nGenerated files:');
 console.log(' proof_simple.json - Zero-knowledge proof');
 console.log(' public_simple.json - Public inputs');
 console.log(' input_simple.json - All inputs (for reference)');

 // Show proof info
 console.log('\n Proof Information:');
 console.log(' Public hash:', publicSignals[0]);
 console.log(' Private preimage: [HIDDEN FOR PRIVACY]');
 console.log(` Proof size: ${JSON.stringify(proof).length} bytes`);
 console.log(` ⏱️ Generation time: ${proofTime}ms`);

 // Verification
 console.log('\n Verifying proof...');
 const vKeyPath = path.join(setupDir, 'verification_key_simple.json');
 if (fs.existsSync(vKeyPath)) {
 const vKey = JSON.parse(fs.readFileSync(vKeyPath));
 const isValid = await snarkjs.groth16.verify(vKey, publicSignals, proof);
 console.log(` Proof is ${isValid ? '�?VALID' : '�?INVALID'}`);
 } else {
 console.log(' ️ Verification key not found, skipping verification');
 }

 console.log('\n Zero-Knowledge Proof Demo Complete!');
 console.log('The prover has demonstrated knowledge of the preimage');
 console.log('without revealing the actual values!');

 } catch (error) {
 console.error('�?Proof generation failed:');
 console.error(error.message);
 process.exit(1);
 }
}

if (require.main === module) {
 main().catch(console.error);
}

module.exports = { main };

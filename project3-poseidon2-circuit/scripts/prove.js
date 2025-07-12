const fs = require('fs');
const path = require('path');
const snarkjs = require('snarkjs');
const { F1Field } = require('ffjavascript');

/*
 * Generate Zero-Knowledge Proof
 *
 * This script generates a Groth16 proof for the Poseidon2 circuit.
 * It demonstrates proving knowledge of a preimage without revealing it.
 */

// BN254 scalar field
const p = "21888242871839275222246405745257275088548364400416034343698204186575808495617";
const Fr = new F1Field(p);

async function main() {
 console.log(' Generating Zero-Knowledge Proof...\n');

 const buildDir = path.join(__dirname, '../build');
 const setupDir = path.join(__dirname, '../setup');
 const proofsDir = path.join(__dirname, '../proofs');

 // Create proofs directory
 if (!fs.existsSync(proofsDir)) {
 fs.mkdirSync(proofsDir, { recursive: true });
 }

 const wasmPath = path.join(buildDir, 'poseidon2_js', 'poseidon2.wasm');
 const zkeyPath = path.join(setupDir, 'poseidon2_final.zkey');

 // Check required files
 if (!fs.existsSync(wasmPath)) {
 console.error(' WASM file not found. Please run compile script first.');
 process.exit(1);
 }

 if (!fs.existsSync(zkeyPath)) {
 console.error(' Proving key not found. Please run setup script first.');
 process.exit(1);
 }

 try {
 console.log('📁 WASM file:', wasmPath);
 console.log('📁 Proving key:', zkeyPath);
 console.log();

 // Generate sample inputs
 console.log(' Generating sample inputs...');

 // Private inputs (preimage) - 2 field elements for t=3 case
 const preimage1 = Fr.random();
 const preimage2 = Fr.random();

 console.log(' Preimage[0]:', Fr.toString(preimage1));
 console.log(' Preimage[1]:', Fr.toString(preimage2));

 // Compute expected hash (this would normally be done separately)
 console.log('\n Computing hash...');

 // For demonstration, we'll use a simple mock hash
 // In practice, you'd use the actual Poseidon2 implementation
 const mockHash = Fr.add(
 Fr.mul(preimage1, Fr.e("2")),
 Fr.mul(preimage2, Fr.e("3"))
 );

 console.log(' Hash value:', Fr.toString(mockHash));

 // Prepare circuit inputs
 const input = {
 "preimage": [Fr.toString(preimage1), Fr.toString(preimage2)],
 "hash": Fr.toString(mockHash)
 };

 // Generate witness
 console.log('\n Generating witness...');
 const witness = await snarkjs.wtns.calculate(input, wasmPath);

 // Generate proof
 console.log(' Generating proof...');
 const startTime = Date.now();

 const { proof, publicSignals } = await snarkjs.groth16.prove(
 zkeyPath,
 witness
 );

 const proofTime = Date.now() - startTime;
 console.log(` Proof generation time: ${proofTime}ms`);

 // Save proof and public inputs
 const proofPath = path.join(proofsDir, 'proof.json');
 const publicPath = path.join(proofsDir, 'public.json');
 const inputPath = path.join(proofsDir, 'input.json');

 fs.writeFileSync(proofPath, JSON.stringify(proof, null, 2));
 fs.writeFileSync(publicPath, JSON.stringify(publicSignals, null, 2));
 fs.writeFileSync(inputPath, JSON.stringify(input, null, 2));

 console.log('\n Proof generated successfully!');
 console.log('\nGenerated files:');
 console.log(' proof.json - Zero-knowledge proof');
 console.log(' public.json - Public inputs');
 console.log(' input.json - All inputs (for reference)');

 // Show proof info
 console.log('\n Proof Information:');
 console.log(' Public hash:', publicSignals[0]);
 console.log(' Private preimage: [HIDDEN]');
 console.log(` 📏 Proof size: ${JSON.stringify(proof).length} bytes`);
 console.log(` Generation time: ${proofTime}ms`);

 // Quick verification
 console.log('\n Quick verification...');
 const vkeyPath = path.join(setupDir, 'verification_key.json');
 if (fs.existsSync(vkeyPath)) {
 const vKey = JSON.parse(fs.readFileSync(vkeyPath));
 const isValid = await snarkjs.groth16.verify(vKey, publicSignals, proof);
 console.log(` Proof is ${isValid ? 'VALID' : 'INVALID'}`);
 } else {
 console.log(' Verification key not found, skipping verification');
 }

 } catch (error) {
 console.error(' Proof generation failed:');
 console.error(error.message);
 process.exit(1);
 }
}

if (require.main === module) {
 main().catch(console.error);
}

module.exports = { main };

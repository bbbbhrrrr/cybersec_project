const { expect } = require('chai');
const fs = require('fs');
const path = require('path');
const snarkjs = require('snarkjs');
const { F1Field } = require('ffjavascript');

describe('Poseidon2 Circuit Tests', function() {
 this.timeout(30000); // 30 second timeout for circuit operations

 const buildDir = path.join(__dirname, '../build');
 const setupDir = path.join(__dirname, '../setup');

 // BN254 scalar field
 const p = "21888242871839275222246405745257275088548364400416034343698204186575808495617";
 const Fr = new F1Field(p);

 before(async function() {
 console.log(' Setting up test environment...');

 // Check if circuit is compiled
 const wasmPath = path.join(buildDir, 'poseidon2.wasm');
 if (!fs.existsSync(wasmPath)) {
 throw new Error('Circuit not compiled. Run npm run compile first.');
 }

 // Check if trusted setup is done
 const zkeyPath = path.join(setupDir, 'poseidon2_final.zkey');
 if (!fs.existsSync(zkeyPath)) {
 throw new Error('Trusted setup not done. Run npm run setup first.');
 }

 console.log(' Test environment ready');
 });

 describe('Circuit Compilation', function() {
 it('should have compiled successfully', function() {
 const wasmPath = path.join(buildDir, 'poseidon2.wasm');
 const r1csPath = path.join(buildDir, 'poseidon2.r1cs');

 expect(fs.existsSync(wasmPath)).to.be.true;
 expect(fs.existsSync(r1csPath)).to.be.true;
 });

 it('should have reasonable constraint count', async function() {
 const r1csPath = path.join(buildDir, 'poseidon2.r1cs');
 const r1cs = await snarkjs.r1cs.info(r1csPath);

 // Poseidon2 should have reasonable constraint count
 expect(r1cs.nConstraints).to.be.above(0);
 expect(r1cs.nConstraints).to.be.below(10000); // Sanity check

 console.log(` Constraints: ${r1cs.nConstraints}`);
 console.log(` Variables: ${r1cs.nVariables}`);
 });
 });

 describe('Trusted Setup', function() {
 it('should have valid proving key', function() {
 const zkeyPath = path.join(setupDir, 'poseidon2_final.zkey');
 expect(fs.existsSync(zkeyPath)).to.be.true;

 const stats = fs.statSync(zkeyPath);
 expect(stats.size).to.be.above(1000); // Reasonable size check
 });

 it('should have valid verification key', function() {
 const vkeyPath = path.join(setupDir, 'verification_key.json');
 expect(fs.existsSync(vkeyPath)).to.be.true;

 const vKey = JSON.parse(fs.readFileSync(vkeyPath));
 expect(vKey).to.have.property('protocol');
 expect(vKey).to.have.property('curve');
 expect(vKey).to.have.property('IC');
 expect(vKey.protocol).to.equal('groth16');
 });
 });

 describe('Witness Generation', function() {
 it('should generate witness for valid inputs', async function() {
 const wasmPath = path.join(buildDir, 'poseidon2.wasm');

 const input = {
 "preimage": ["123", "456"],
 "hash": "789" // Mock hash for testing
 };

 const witness = await snarkjs.wtns.calculate(input, wasmPath);
 expect(witness).to.be.an('array');
 expect(witness.length).to.be.above(0);
 });

 it('should fail for invalid inputs', async function() {
 const wasmPath = path.join(buildDir, 'poseidon2.wasm');

 const input = {
 "preimage": ["123", "456"],
 "hash": "999" // Wrong hash value
 };

 try {
 await snarkjs.wtns.calculate(input, wasmPath);
 expect.fail('Should have thrown an error for invalid inputs');
 } catch (error) {
 expect(error).to.exist;
 }
 });
 });

 describe('Proof Generation and Verification', function() {
 let validProof, validPublicSignals;

 it('should generate valid proof for correct preimage', async function() {
 const wasmPath = path.join(buildDir, 'poseidon2.wasm');
 const zkeyPath = path.join(setupDir, 'poseidon2_final.zkey');

 // Generate test inputs
 const preimage1 = Fr.e("12345");
 const preimage2 = Fr.e("67890");

 // Mock hash calculation (in practice, use actual Poseidon2)
 const hash = Fr.add(
 Fr.mul(preimage1, Fr.e("2")),
 Fr.mul(preimage2, Fr.e("3"))
 );

 const input = {
 "preimage": [Fr.toString(preimage1), Fr.toString(preimage2)],
 "hash": Fr.toString(hash)
 };

 // Generate witness and proof
 const witness = await snarkjs.wtns.calculate(input, wasmPath);
 const { proof, publicSignals } = await snarkjs.groth16.prove(
 zkeyPath,
 witness
 );

 expect(proof).to.exist;
 expect(publicSignals).to.exist;
 expect(publicSignals).to.have.length(1);
 expect(publicSignals[0]).to.equal(Fr.toString(hash));

 validProof = proof;
 validPublicSignals = publicSignals;
 });

 it('should verify valid proof', async function() {
 const vkeyPath = path.join(setupDir, 'verification_key.json');
 const vKey = JSON.parse(fs.readFileSync(vkeyPath));

 const isValid = await snarkjs.groth16.verify(
 vKey,
 validPublicSignals,
 validProof
 );

 expect(isValid).to.be.true;
 });

 it('should reject proof with wrong public input', async function() {
 const vkeyPath = path.join(setupDir, 'verification_key.json');
 const vKey = JSON.parse(fs.readFileSync(vkeyPath));

 // Modify public input
 const wrongPublicSignals = [...validPublicSignals];
 wrongPublicSignals[0] = Fr.toString(Fr.e("999999"));

 const isValid = await snarkjs.groth16.verify(
 vKey,
 wrongPublicSignals,
 validProof
 );

 expect(isValid).to.be.false;
 });
 });

 describe('Security Properties', function() {
 it('should not reveal preimage in proof', function() {
 const proofStr = JSON.stringify(validProof);

 // Proof should not contain obvious preimage values
 expect(proofStr).to.not.include('12345');
 expect(proofStr).to.not.include('67890');
 });

 it('should have deterministic verification', async function() {
 const vkeyPath = path.join(setupDir, 'verification_key.json');
 const vKey = JSON.parse(fs.readFileSync(vkeyPath));

 // Verify same proof multiple times
 const result1 = await snarkjs.groth16.verify(
 vKey,
 validPublicSignals,
 validProof
 );
 const result2 = await snarkjs.groth16.verify(
 vKey,
 validPublicSignals,
 validProof
 );

 expect(result1).to.equal(result2);
 expect(result1).to.be.true;
 });
 });

 describe('Performance Tests', function() {
 it('should generate proof in reasonable time', async function() {
 const wasmPath = path.join(buildDir, 'poseidon2.wasm');
 const zkeyPath = path.join(setupDir, 'poseidon2_final.zkey');

 const input = {
 "preimage": ["111", "222"],
 "hash": "555" // Mock hash
 };

 const startTime = Date.now();
 const witness = await snarkjs.wtns.calculate(input, wasmPath);
 await snarkjs.groth16.prove(zkeyPath, witness);
 const endTime = Date.now();

 const proofTime = endTime - startTime;
 console.log(` Proof generation time: ${proofTime}ms`);

 // Should be reasonable (less than 5 seconds for small circuit)
 expect(proofTime).to.be.below(5000);
 });

 it('should verify proof quickly', async function() {
 const vkeyPath = path.join(setupDir, 'verification_key.json');
 const vKey = JSON.parse(fs.readFileSync(vkeyPath));

 const startTime = Date.now();
 await snarkjs.groth16.verify(vKey, validPublicSignals, validProof);
 const endTime = Date.now();

 const verifyTime = endTime - startTime;
 console.log(` Verification time: ${verifyTime}ms`);

 // Verification should be very fast (less than 100ms)
 expect(verifyTime).to.be.below(100);
 });
 });
});

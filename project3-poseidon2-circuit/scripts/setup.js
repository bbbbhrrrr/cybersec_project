const fs = require('fs');
const path = require('path');
const snarkjs = require('snarkjs');

/*
 * Setup Trusted Setup for Groth16
 *
 * This script performs the trusted setup ceremony
 * required for Groth16 proof system.
 */

async function main() {
 console.log(' Setting up Groth16 Trusted Setup...\n');

 // Create setup directory
 const setupDir = path.join(__dirname, '../setup');
 if (!fs.existsSync(setupDir)) {
 fs.mkdirSync(setupDir, { recursive: true });
 }

 const buildDir = path.join(__dirname, '../build');
 const r1csPath = path.join(buildDir, 'poseidon2.r1cs');

 if (!fs.existsSync(r1csPath)) {
 console.error(' R1CS file not found. Please run compile script first.');
 process.exit(1);
 }

 try {
 console.log('📁 R1CS file:', r1csPath);
 console.log('📁 Setup directory:', setupDir);
 console.log();

 // Phase 1: Powers of Tau ceremony
 console.log(' Phase 1: Powers of Tau ceremony...');
 const ptauPath = path.join(setupDir, 'powersOfTau.ptau');

 // Start new ceremony (for small circuits, we can use a small power)
 console.log(' Starting new ceremony...');
 await snarkjs.powersOfTau.newAccumulator(
 "bn128",
 12, // 2^12 = 4096 constraints (adjust based on circuit size)
 ptauPath
 );

 console.log(' Contributing to ceremony...');
 const ptauPath1 = path.join(setupDir, 'powersOfTau1.ptau');
 await snarkjs.powersOfTau.contribute(
 ptauPath,
 ptauPath1,
 "first contribution",
 "random entropy"
 );

 console.log(' Preparing phase 2...');
 const ptauFinalPath = path.join(setupDir, 'powersOfTauFinal.ptau');
 await snarkjs.powersOfTau.preparePhase2(
 ptauPath1,
 ptauFinalPath
 );

 // Phase 2: Circuit-specific setup
 console.log(' Phase 2: Circuit-specific setup...');
 const zkeyPath = path.join(setupDir, 'poseidon2.zkey');

 console.log(' 🔑 Generating initial zkey...');
 await snarkjs.groth16.setup(
 r1csPath,
 ptauFinalPath,
 zkeyPath
 );

 console.log(' 🔑 Contributing to phase 2...');
 const zkeyFinalPath = path.join(setupDir, 'poseidon2_final.zkey');
 await snarkjs.zKey.contribute(
 zkeyPath,
 zkeyFinalPath,
 "first contribution",
 "random entropy for phase 2"
 );

 // Export verification key
 console.log(' 📤 Exporting verification key...');
 const vkeyPath = path.join(setupDir, 'verification_key.json');
 const vKey = await snarkjs.zKey.exportVerificationKey(zkeyFinalPath);
 fs.writeFileSync(vkeyPath, JSON.stringify(vKey, null, 2));

 console.log('\n Trusted setup completed!');
 console.log('\nGenerated files:');
 console.log(' 🔑 poseidon2_final.zkey - Final proving key');
 console.log(' 🔓 verification_key.json - Verification key');
 console.log(' powersOfTauFinal.ptau - Universal setup');

 // Show verification key info
 console.log('\n Verification Key Info:');
 console.log(` 📐 Curve: ${vKey.curve}`);
 console.log(` 🔢 IC length: ${vKey.IC.length}`);
 console.log(` Protocol: ${vKey.protocol}`);

 // Clean up intermediate files
 console.log('\n🧹 Cleaning up intermediate files...');
 [ptauPath, ptauPath1, zkeyPath].forEach(file => {
 if (fs.existsSync(file)) {
 fs.unlinkSync(file);
 console.log(` Removed ${path.basename(file)}`);
 }
 });

 } catch (error) {
 console.error(' Setup failed:');
 console.error(error.message);
 process.exit(1);
 }
}

if (require.main === module) {
 main().catch(console.error);
}

module.exports = { main };

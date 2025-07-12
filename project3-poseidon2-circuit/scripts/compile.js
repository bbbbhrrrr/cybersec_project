const fs = require('fs');
const path = require('path');
const { execSync } = require('child_process');

/*
 * Compile Poseidon2 Circuit
 * 
 * This script compiles the Circom circuit and generates
 * the necessary files for proof generation.
 */

async function main() {
    console.log('🔨 Compiling Poseidon2 Circuit...\n');
    
    // Create build directory
    const buildDir = path.join(__dirname, '../build');
    if (!fs.existsSync(buildDir)) {
        fs.mkdirSync(buildDir, { recursive: true });
    }
    
    const circuitPath = path.join(__dirname, '../circuits/poseidon2.circom');
    const outputDir = buildDir;
    
    try {
        console.log('📁 Circuit file:', circuitPath);
        console.log('📁 Output directory:', outputDir);
        console.log();
        
        // Compile circuit
        console.log('⚙️  Compiling circuit with Circom...');
        const compileCmd = `circom "${circuitPath}" --r1cs --wasm --sym --c -o "${outputDir}"`;
        console.log('Command:', compileCmd);
        
        execSync(compileCmd, { stdio: 'inherit' });
        
        console.log('\n✅ Circuit compilation completed!');
        console.log('\nGenerated files:');
        console.log('  📄 poseidon2.r1cs - R1CS constraint system');
        console.log('  📄 poseidon2.wasm - WebAssembly witness generator');
        console.log('  📄 poseidon2.sym - Symbol table');
        console.log('  📄 poseidon2.cpp - C++ witness generator');
        
        // Show circuit statistics
        console.log('\n📊 Circuit Statistics:');
        const r1csPath = path.join(outputDir, 'poseidon2.r1cs');
        if (fs.existsSync(r1csPath)) {
            try {
                const snarkjs = require('snarkjs');
                const r1cs = await snarkjs.r1cs.info(r1csPath);
                console.log(`  🔢 Constraints: ${r1cs.nConstraints}`);
                console.log(`  🔢 Variables: ${r1cs.nVariables}`);
                console.log(`  🔢 Public inputs: ${r1cs.nPublicInputs}`);
                console.log(`  🔢 Private inputs: ${r1cs.nPrivateInputs}`);
                console.log(`  🔢 Outputs: ${r1cs.nOutputs}`);
            } catch (err) {
                console.log('  ⚠️  Unable to read circuit statistics');
            }
        }
        
    } catch (error) {
        console.error('❌ Compilation failed:');
        console.error(error.message);
        process.exit(1);
    }
}

if (require.main === module) {
    main().catch(console.error);
}

module.exports = { main };

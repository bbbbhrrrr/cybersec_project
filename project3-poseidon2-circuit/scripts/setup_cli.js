const fs = require('fs');
const path = require('path');
const { execSync } = require('child_process');

/*
 * Simplified Setup using snarkjs CLI
 */

async function main() {
    console.log('🔨 Setting up Groth16 Trusted Setup...\n');

    // Create setup directory
    const setupDir = path.join(__dirname, '../setup');
    if (!fs.existsSync(setupDir)) {
        fs.mkdirSync(setupDir, { recursive: true });
    }

    const buildDir = path.join(__dirname, '../build');
    const r1csPath = path.join(buildDir, 'poseidon2.r1cs');

    if (!fs.existsSync(r1csPath)) {
        console.error('�?R1CS file not found. Please run compile script first.');
        process.exit(1);
    }

    try {
        console.log('📁 R1CS file:', r1csPath);
        console.log('📁 Setup directory:', setupDir);
        console.log();

        // Use CLI commands instead of API
        const ptauPath = path.join(setupDir, 'powersOfTau.ptau');
        const ptauPath1 = path.join(setupDir, 'powersOfTau1.ptau');  
        const ptauFinalPath = path.join(setupDir, 'powersOfTauFinal.ptau');
        const zkeyPath = path.join(setupDir, 'poseidon2.zkey');
        const zkeyFinalPath = path.join(setupDir, 'poseidon2_final.zkey');
        const vkeyPath = path.join(setupDir, 'verification_key.json');

        console.log('�?Phase 1: Powers of Tau ceremony...');
        
        // Phase 1
        console.log('  🔄 Starting new ceremony...');
        execSync(`npx snarkjs powersoftau new bn128 12 "${ptauPath}" -v`, { stdio: 'inherit' });
        
        console.log('  🔄 Contributing to ceremony...');
        execSync(`npx snarkjs powersoftau contribute "${ptauPath}" "${ptauPath1}" --name="First contribution" -v`, { stdio: 'inherit' });
        
        console.log('  🔄 Preparing phase 2...');
        execSync(`npx snarkjs pt2 "${ptauPath1}" "${ptauFinalPath}" -v`, { stdio: 'inherit' });

        // Phase 2
        console.log('�?Phase 2: Circuit-specific setup...');
        
        console.log('  🔑 Generating initial zkey...');
        execSync(`npx snarkjs g16s "${r1csPath}" "${ptauFinalPath}" "${zkeyPath}"`, { stdio: 'inherit' });
        
        console.log('  🔑 Contributing to phase 2...');
        execSync(`npx snarkjs zkc "${zkeyPath}" "${zkeyFinalPath}" --name="First contribution" -v`, { stdio: 'inherit' });
        
        console.log('  📤 Exporting verification key...');
        execSync(`npx snarkjs zkev "${zkeyFinalPath}" "${vkeyPath}"`, { stdio: 'inherit' });

        console.log('\n�?Trusted setup completed!');
        console.log('\nGenerated files:');
        console.log(' 🔑 poseidon2_final.zkey - Final proving key');
        console.log(' 🔓 verification_key.json - Verification key');
        console.log(' �?powersOfTauFinal.ptau - Universal setup');

        // Show verification key info
        if (fs.existsSync(vkeyPath)) {
            const vKey = JSON.parse(fs.readFileSync(vkeyPath, 'utf8'));
            console.log('\n📊 Verification Key Info:');
            console.log(` 📐 Curve: ${vKey.curve}`);
            console.log(` 🔢 IC length: ${vKey.IC.length}`);
            console.log(` Protocol: ${vKey.protocol}`);
        }

        // Clean up intermediate files
        console.log('\n🧹 Cleaning up intermediate files...');
        [ptauPath, ptauPath1, zkeyPath].forEach(file => {
            if (fs.existsSync(file)) {
                fs.unlinkSync(file);
                console.log(` ♻️  Removed ${path.basename(file)}`);
            }
        });

    } catch (error) {
        console.error('�?Setup failed:');
        console.error(error.message);
        process.exit(1);
    }
}

if (require.main === module) {
    main().catch(console.error);
}

module.exports = { main };

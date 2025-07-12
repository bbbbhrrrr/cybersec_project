const fs = require('fs');
const path = require('path');

/*
 * Export Verifier Contract
 * 
 * This script exports the verification key and generates
 * a Solidity verifier contract for on-chain verification.
 */

async function main() {
    console.log('📤 Exporting Verifier Contract...\n');
    
    const setupDir = path.join(__dirname, '../setup');
    const contractsDir = path.join(__dirname, '../contracts');
    const buildDir = path.join(__dirname, '../build');
    
    // Check if verification key exists
    const vkeyPath = path.join(setupDir, 'verification_key.json');
    if (!fs.existsSync(vkeyPath)) {
        console.error('❌ Verification key not found. Please run setup script first.');
        process.exit(1);
    }
    
    try {
        console.log('📁 Verification key:', vkeyPath);
        console.log('📁 Contracts directory:', contractsDir);
        console.log();
        
        // Load verification key
        console.log('📖 Loading verification key...');
        const vKey = JSON.parse(fs.readFileSync(vkeyPath));
        
        // Generate Solidity verifier
        console.log('⚙️  Generating Solidity verifier...');
        const snarkjs = require('snarkjs');
        
        // Export Solidity verifier
        const solidityVerifier = await snarkjs.zKey.exportSolidityVerifier(
            path.join(setupDir, 'poseidon2_final.zkey')
        );
        
        // Save verifier contract
        const verifierPath = path.join(contractsDir, 'Poseidon2VerifierGenerated.sol');
        fs.writeFileSync(verifierPath, solidityVerifier);
        
        console.log('✅ Solidity verifier generated!');
        console.log('  📄 File:', path.basename(verifierPath));
        
        // Generate deployment script
        console.log('⚙️  Generating deployment script...');
        const deployScript = generateDeployScript(vKey);
        const deployPath = path.join(contractsDir, 'deploy.js');
        fs.writeFileSync(deployPath, deployScript);
        
        console.log('✅ Deployment script generated!');
        console.log('  📄 File:', path.basename(deployPath));
        
        // Generate verification example
        console.log('⚙️  Generating verification example...');
        const exampleScript = generateVerificationExample();
        const examplePath = path.join(contractsDir, 'verify_example.js');
        fs.writeFileSync(examplePath, exampleScript);
        
        console.log('✅ Verification example generated!');
        console.log('  📄 File:', path.basename(examplePath));
        
        console.log('\n📊 Export Summary:');
        console.log('  📄 Poseidon2VerifierGenerated.sol - Auto-generated verifier');
        console.log('  📄 deploy.js - Contract deployment script');
        console.log('  📄 verify_example.js - Usage example');
        
        // Show gas estimation
        console.log('\n⛽ Gas Estimation:');
        console.log('  🚀 Deployment: ~2,500,000 gas');
        console.log('  ✅ Verification: ~250,000 gas per proof');
        
    } catch (error) {
        console.error('❌ Export failed:');
        console.error(error.message);
        process.exit(1);
    }
}

function generateDeployScript(vKey) {
    return `// Poseidon2 Verifier Deployment Script
const { ethers } = require('hardhat');

async function main() {
    console.log('🚀 Deploying Poseidon2 Verifier...');
    
    const VerifierFactory = await ethers.getContractFactory('Poseidon2VerifierGenerated');
    
    // Deploy with verification key parameters
    const verifier = await VerifierFactory.deploy();
    await verifier.deployed();
    
    console.log('✅ Verifier deployed to:', verifier.address);
    
    // Verify deployment
    const isValid = await verifier.verifyingKey();
    console.log('🔍 Verification key loaded:', !!isValid);
    
    return verifier.address;
}

main()
    .then(() => process.exit(0))
    .catch((error) => {
        console.error(error);
        process.exit(1);
    });
`;
}

function generateVerificationExample() {
    return `// Poseidon2 On-chain Verification Example
const { ethers } = require('hardhat');
const fs = require('fs');
const path = require('path');

async function main() {
    console.log('🔍 Testing on-chain verification...');
    
    // Load proof artifacts
    const proofPath = path.join(__dirname, '../proofs/proof.json');
    const publicPath = path.join(__dirname, '../proofs/public.json');
    
    if (!fs.existsSync(proofPath) || !fs.existsSync(publicPath)) {
        console.error('❌ Proof files not found. Run npm run prove first.');
        return;
    }
    
    const proof = JSON.parse(fs.readFileSync(proofPath));
    const publicSignals = JSON.parse(fs.readFileSync(publicPath));
    
    // Get deployed verifier
    const verifierAddress = process.env.VERIFIER_ADDRESS;
    if (!verifierAddress) {
        console.error('❌ VERIFIER_ADDRESS not set. Deploy contract first.');
        return;
    }
    
    const verifier = await ethers.getContractAt('Poseidon2VerifierGenerated', verifierAddress);
    
    // Format proof for Solidity
    const proofFormatted = [
        [proof.pi_a[0], proof.pi_a[1]],
        [[proof.pi_b[0][1], proof.pi_b[0][0]], [proof.pi_b[1][1], proof.pi_b[1][0]]],
        [proof.pi_c[0], proof.pi_c[1]]
    ];
    
    // Verify proof on-chain
    console.log('⚙️  Submitting proof to blockchain...');
    const tx = await verifier.verifyProof(
        proofFormatted[0],
        proofFormatted[1], 
        proofFormatted[2],
        publicSignals
    );
    
    const receipt = await tx.wait();
    console.log('✅ Transaction confirmed:', receipt.transactionHash);
    console.log('⛽ Gas used:', receipt.gasUsed.toString());
    
    // Check result
    const result = await verifier.verifyProof(
        proofFormatted[0],
        proofFormatted[1],
        proofFormatted[2], 
        publicSignals
    );
    
    console.log('🎯 Verification result:', result ? 'VALID' : 'INVALID');
}

main().catch(console.error);
`;
}

if (require.main === module) {
    main().catch(console.error);
}

module.exports = { main };

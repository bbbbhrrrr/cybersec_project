pragma circom 2.1.6;

include "./utils/poseidon2_constants.circom";
include "./utils/poseidon2_round.circom";

/*
 * Poseidon2 Hash Function with parameters (n,t,d) = (256,2,5)
 * 
 * Based on Table 1 of https://eprint.iacr.org/2023/323.pdf:
 * - Field size (n): 256 bits (BN254 scalar field ~254 bits)
 * - State size (t): 2 elements  
 * - S-box degree (d): 5
 * - Full rounds (RF): 8
 * - Partial rounds (RP): 56
 * - Total rounds: 64
 * 
 * This version has a smaller state size (t=2) which means:
 * - Only 1 input element (rate=1, capacity=1)
 * - Faster computation but less throughput
 * - Suitable for single-element hashing
 */

template Poseidon2T2() {
    // Input: 1 field element to hash (since t=2, rate=1)
    signal input input0;
    
    // Output: hash value
    signal output hash;
    
    // State size t=2
    var t = 2;
    
    // Round configuration for t=2 from Table 1
    var RF = 8;  // Full rounds
    var RP = 56; // Partial rounds
    var totalRounds = RF + RP; // 64 total rounds
    
    // Initialize state: [0, input0]
    // Capacity=1, Rate=1 sponge construction
    signal state[totalRounds + 1][t];
    state[0][0] <== 0;      // Capacity (domain separator)
    state[0][1] <== input0; // Single input
    
    // Apply rounds
    component rounds[totalRounds];
    component constants = Poseidon2Constants(t, totalRounds);
    
    for (var round = 0; round < totalRounds; round++) {
        // Full rounds: first RF/2 and last RF/2
        // Partial rounds: middle RP rounds
        var isFullRound = (round < RF/2) || (round >= RF/2 + RP);
        
        rounds[round] = Poseidon2Round(t, isFullRound);
        
        // Connect state and constants
        for (var i = 0; i < t; i++) {
            rounds[round].state[i] <== state[round][i];
            rounds[round].roundConstant[i] <== constants.constants[round][i];
        }
        
        // Update state
        for (var i = 0; i < t; i++) {
            state[round + 1][i] <== rounds[round].out[i];
        }
    }
    
    // Output the second element (rate part)
    hash <== state[totalRounds][1];
}

/*
 * Zero-Knowledge Proof Circuit for single preimage with t=2
 * 
 * Public input: hash value
 * Private input: single field element preimage
 */
template Poseidon2T2Preimage() {
    // Private input (witness) - single field element
    signal private input preimage;
    
    // Public input (statement)
    signal input expectedHash;
    
    // Compute hash of preimage
    component hasher = Poseidon2T2();
    hasher.input0 <== preimage;
    
    // Constraint: computed hash must equal expected hash
    expectedHash === hasher.hash;
}

/*
 * Dual-input version that hashes two elements sequentially with t=2
 * This provides similar functionality to t=3 but using t=2 parameters
 */
template Poseidon2T2Dual() {
    // Input: 2 field elements to hash
    signal input input0;
    signal input input1;
    
    // Output: hash value
    signal output hash;
    
    // First hash with input0
    component hash1 = Poseidon2T2();
    hash1.input0 <== input0;
    
    // Second hash with result and input1  
    component hash2 = Poseidon2T2();
    hash2.input0 <== hash1.hash + input1; // Combine intermediate hash with second input
    
    hash <== hash2.hash;
}

/*
 * Zero-Knowledge Proof Circuit for dual preimage with t=2
 * 
 * Public input: hash value
 * Private input: two field elements as preimage
 */
template Poseidon2T2DualPreimage() {
    // Private inputs (witness)
    signal private input preimage0;
    signal private input preimage1;
    
    // Public input (statement)
    signal input expectedHash;
    
    // Compute hash of preimage pair
    component hasher = Poseidon2T2Dual();
    hasher.input0 <== preimage0;
    hasher.input1 <== preimage1;
    
    // Constraint: computed hash must equal expected hash
    expectedHash === hasher.hash;
}

// Main component - can switch between single and dual input versions
component main = Poseidon2T2DualPreimage();

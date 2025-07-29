pragma circom 2.1.6;

include "./utils/poseidon2_constants.circom";
include "./utils/poseidon2_round.circom";

/*
 * Poseidon2 Hash Function - Simple and Optimized Version
 * 
 * Based on "Poseidon2: A Faster Version of the Poseidon Hash Function"
 * https://eprint.iacr.org/2023/323.pdf
 * 
 * This implementation uses parameters (n,t,d) = (256,3,5):
 * - n: 256-bit field (BN254 scalar field ~254 bits)
 * - t: 3 (state size)
 * - d: 5 (S-box degree, x^5)
 * 
 * Round configuration from Table 1:
 * - RF: 8 full rounds (4 + 4)  
 * - RP: 57 partial rounds
 * - Total: 65 rounds
 */

template Poseidon2Simple() {
    // Input: 2 field elements to hash
    signal input input0;
    signal input input1;
    
    // Output: hash value (single field element)
    signal output hash;
    
    // State size t=3
    var t = 3;
    
    // Round configuration for t=3 from Table 1
    var RF = 8;  // Full rounds
    var RP = 57; // Partial rounds  
    var totalRounds = RF + RP; // 65 total rounds
    
    // Initialize state: [0, input0, input1]
    // Following sponge construction with capacity=1, rate=2
    signal state[totalRounds + 1][t];
    state[0][0] <== 0;      // Capacity (domain separator)
    state[0][1] <== input0; // First input
    state[0][2] <== input1; // Second input
    
    // Apply rounds
    component rounds[totalRounds];
    component constants = Poseidon2Constants(t, totalRounds);
    
    for (var round = 0; round < totalRounds; round++) {
        // Determine if this is a full round
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
    
    // Output the second element (rate part) as hash
    // This follows the sponge construction pattern
    hash <== state[totalRounds][1];
}

/*
 * Zero-Knowledge Proof Circuit for Poseidon2 Preimage
 * 
 * Public input: hash value
 * Private input: preimage (2 field elements)
 * 
 * Proves knowledge of preimage without revealing it
 */
template Poseidon2Preimage() {
    // Private inputs (witness)
    signal private input preimage0;
    signal private input preimage1;
    
    // Public input (statement)
    signal input expectedHash;
    
    // Compute hash of preimage
    component hasher = Poseidon2Simple();
    hasher.input0 <== preimage0;
    hasher.input1 <== preimage1;
    
    // Constraint: computed hash must equal expected hash
    expectedHash === hasher.hash;
}

// Main component for Groth16 proof generation
component main = Poseidon2Preimage();

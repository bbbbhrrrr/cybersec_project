pragma circom 2.1.6;

include "./utils/poseidon2_constants.circom";
include "./utils/poseidon2_round.circom";

/*
 * Poseidon2 Hash Function Circuit Implementation
 * 
 * Parameters from Table 1 of https://eprint.iacr.org/2023/323.pdf:
 * - Field size (n): 256 bits (BN254 scalar field ~254 bits)
 * - State size (t): 2 or 3 elements
 * - S-box degree (d): 5
 * - Round numbers from Table 1:
 *   - For t=2: RF=8, RP=56, Total=64
 *   - For t=3: RF=8, RP=57, Total=65
 *
 * This circuit implements the Poseidon2 permutation and hash function
 * optimized for the BN254 curve with correct parameters from the paper.
 */

template Poseidon2Permutation(t) {
    assert(t == 2 || t == 3);
    
    signal input state[t];
    signal output out[t];
    
    // Round parameters from Table 1 of the Poseidon2 paper
    var RF = 8;  // Full rounds (same for both t=2 and t=3)
    var RP = t == 2 ? 56 : 57; // Partial rounds from Table 1
    var totalRounds = RF + RP;
    
    component rounds[totalRounds];
    component constants = Poseidon2Constants(t, totalRounds);
    
    // Initial state
    signal stateRounds[totalRounds + 1][t];
    for (var i = 0; i < t; i++) {
        stateRounds[0][i] <== state[i];
    }
    
    // Apply rounds
    for (var round = 0; round < totalRounds; round++) {
        // First RF/2 rounds are full, middle RP rounds are partial, last RF/2 rounds are full
        var isFullRound = (round < RF/2) || (round >= RF/2 + RP);
        
        rounds[round] = Poseidon2Round(t, isFullRound);
        
        for (var i = 0; i < t; i++) {
            rounds[round].state[i] <== stateRounds[round][i];
            rounds[round].roundConstant[i] <== constants.constants[round][i];
        }
        
        for (var i = 0; i < t; i++) {
            stateRounds[round + 1][i] <== rounds[round].out[i];
        }
    }
    
    // Output final state
    for (var i = 0; i < t; i++) {
        out[i] <== stateRounds[totalRounds][i];
    }
}

/*
 * Poseidon2 Hash Function Template
 * Implements the sponge construction with rate=t-1, capacity=1
 */
template Poseidon2Hash(t) {
    assert(t == 2 || t == 3);
    
    signal input inputs[t-1];  // t-1 inputs, with capacity 1
    signal output out;
    
    component permutation = Poseidon2Permutation(t);
    
    // Initialize state: [0, input1, input2, ...] for t=3
    // or [0, input1] for t=2
    permutation.state[0] <== 0;  // Capacity element (domain separator)
    for (var i = 0; i < t-1; i++) {
        permutation.state[i+1] <== inputs[i];
    }
    
    // Output second element (first element of rate part)
    // This follows standard sponge construction
    out <== permutation.out[1];
}

/*
 * Main circuit template for zero-knowledge proof
 * Public input: hash value
 * Private input: preimage (2 field elements for t=3)
 * 
 * Proves knowledge of preimage without revealing it
 */
template Poseidon2ZK() {
    // Private inputs (preimage) - witness
    signal private input preimage[2];  // Using t=3, so 2 field elements as input
    
    // Public inputs (hash value) - statement
    signal input hash;
    
    // Compute hash of preimage
    component hasher = Poseidon2Hash(3);
    hasher.inputs[0] <== preimage[0];
    hasher.inputs[1] <== preimage[1];
    
    // Constrain computed hash equals public hash
    hash === hasher.out;
}

// Main component for circuit compilation and proof generation
component main = Poseidon2ZK();

// Main component
component main = Poseidon2ZK();

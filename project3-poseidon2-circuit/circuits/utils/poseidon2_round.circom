pragma circom 2.1.6;

/*
 * Poseidon2 Round Function Implementation
 * 
 * Implements one round of the Poseidon2 permutation based on:
 * https://eprint.iacr.org/2023/323.pdf
 * 
 * Round function consists of:
 * 1. Add round constants  
 * 2. Apply S-box (x^d where d=5)
 * 3. Apply linear layer (improved MDS matrix from Poseidon2)
 */

template Poseidon2Round(t, isFullRound) {
    signal input state[t];
    signal input roundConstant[t];
    signal output out[t];
    
    // Step 1: Add round constants
    signal afterConstants[t];
    for (var i = 0; i < t; i++) {
        afterConstants[i] <== state[i] + roundConstant[i];
    }
    
    // Step 2: Apply S-box (x^5)
    signal afterSbox[t];
    
    if (isFullRound) {
        // Full round: apply S-box to all elements
        for (var i = 0; i < t; i++) {
            afterSbox[i] <== PowerFive()(afterConstants[i]);
        }
    } else {
        // Partial round: apply S-box only to first element
        afterSbox[0] <== PowerFive()(afterConstants[0]);
        for (var i = 1; i < t; i++) {
            afterSbox[i] <== afterConstants[i];
        }
    }
    
    // Step 3: Apply improved linear layer from Poseidon2
    if (t == 2) {
        // Improved matrix for t=2 (from Poseidon2 paper)
        // Matrix: [[2, 1], [1, 2]]
        out[0] <== 2 * afterSbox[0] + afterSbox[1];
        out[1] <== afterSbox[0] + 2 * afterSbox[1];
    } else if (t == 3) {
        // Improved matrix for t=3 (from Poseidon2 paper)
        // Matrix: [[2, 1, 1], [1, 2, 1], [1, 1, 3]]
        out[0] <== 2 * afterSbox[0] + afterSbox[1] + afterSbox[2];
        out[1] <== afterSbox[0] + 2 * afterSbox[1] + afterSbox[2];
        out[2] <== afterSbox[0] + afterSbox[1] + 3 * afterSbox[2];
    }
}

/*
 * S-box: x^5 over the base field
 * Optimized for minimal constraints in zero-knowledge circuits
 */
template PowerFive() {
    signal input in;
    signal output out;
    
    signal x2 <== in * in;
    signal x4 <== x2 * x2;
    out <== x4 * in;
}

pragma circom 2.1.6;

/*
 * Poseidon2 Round Function Implementation
 * 
 * Implements one round of the Poseidon2 permutation:
 * 1. Add round constants  
 * 2. Apply S-box (x^5 for full rounds, only first element for partial rounds)
 * 3. Apply MDS matrix multiplication
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
    
    // Step 2: Apply S-box
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
    
    // Step 3: Apply MDS matrix
    if (t == 2) {
        component mds = MDSMatrix2();
        mds.in[0] <== afterSbox[0];
        mds.in[1] <== afterSbox[1];
        out[0] <== mds.out[0];
        out[1] <== mds.out[1];
    } else if (t == 3) {
        component mds = MDSMatrix3();
        mds.in[0] <== afterSbox[0];
        mds.in[1] <== afterSbox[1];
        mds.in[2] <== afterSbox[2];
        out[0] <== mds.out[0];
        out[1] <== mds.out[1];
        out[2] <== mds.out[2];
    }
}

/*
 * S-box: x^5 over the base field
 */
template PowerFive() {
    signal input in;
    signal output out;
    
    signal x2 <== in * in;
    signal x4 <== x2 * x2;
    out <== x4 * in;
}

/*
 * MDS Matrix for t=2
 * Optimized matrix from Poseidon2 specification
 */
template MDSMatrix2() {
    signal input in[2];
    signal output out[2];
    
    // MDS matrix values for t=2 (from Poseidon2 paper)
    // [2, 1]
    // [1, 2]
    out[0] <== 2 * in[0] + in[1];
    out[1] <== in[0] + 2 * in[1];
}

/*
 * MDS Matrix for t=3  
 * Optimized matrix from Poseidon2 specification
 */
template MDSMatrix3() {
    signal input in[3];
    signal output out[3];
    
    // MDS matrix values for t=3 (from Poseidon2 paper)
    // [2, 1, 1]
    // [1, 2, 1] 
    // [1, 1, 3]
    out[0] <== 2 * in[0] + in[1] + in[2];
    out[1] <== in[0] + 2 * in[1] + in[2];
    out[2] <== in[0] + in[1] + 3 * in[2];
}

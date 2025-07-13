pragma circom 2.1.6;

/*
 * Simplified Poseidon2 Implementation for Demonstration
 * This version uses smaller constants and simplified logic
 */

template SimplePoseidon2() {
    signal input preimage[2];
    signal input hash;
    
    // Simple hash computation: (preimage[0] + preimage[1])^5 + 12345
    signal sum <== preimage[0] + preimage[1];
    signal sum2 <== sum * sum;
    signal sum4 <== sum2 * sum2;
    signal sum5 <== sum4 * sum;
    signal computed_hash <== sum5 + 12345;
    
    // Constraint: computed hash must equal public hash
    hash === computed_hash;
}

component main = SimplePoseidon2();

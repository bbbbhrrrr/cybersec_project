// SPDX-License-Identifier: MIT
pragma solidity ^0.8.0;

/*
 * Poseidon2 Verifier Contract
 * 
 * This contract verifies Groth16 proofs for the Poseidon2 circuit.
 * It enables on-chain verification of zero-knowledge proofs.
 */

contract Poseidon2Verifier {
    using Pairing for *;
    
    struct VerifyingKey {
        Pairing.G1Point alpha;
        Pairing.G2Point beta;
        Pairing.G2Point gamma;
        Pairing.G2Point delta;
        Pairing.G1Point[] gamma_abc;
    }
    
    struct Proof {
        Pairing.G1Point a;
        Pairing.G2Point b;
        Pairing.G1Point c;
    }
    
    VerifyingKey verifyingKey;
    
    event ProofVerified(address indexed prover, uint256 hash, bool isValid);
    
    constructor(
        uint256[2] memory _alpha,
        uint256[2][2] memory _beta,
        uint256[2][2] memory _gamma,
        uint256[2][2] memory _delta,
        uint256[2][] memory _gamma_abc
    ) {
        verifyingKey.alpha = Pairing.G1Point(_alpha[0], _alpha[1]);
        verifyingKey.beta = Pairing.G2Point([_beta[0][0], _beta[0][1]], [_beta[1][0], _beta[1][1]]);
        verifyingKey.gamma = Pairing.G2Point([_gamma[0][0], _gamma[0][1]], [_gamma[1][0], _gamma[1][1]]);
        verifyingKey.delta = Pairing.G2Point([_delta[0][0], _delta[0][1]], [_delta[1][0], _delta[1][1]]);
        
        for (uint i = 0; i < _gamma_abc.length; i++) {
            verifyingKey.gamma_abc.push(Pairing.G1Point(_gamma_abc[i][0], _gamma_abc[i][1]));
        }
    }
    
    function verifyProof(
        uint[2] memory _pA,
        uint[2][2] memory _pB,
        uint[2] memory _pC,
        uint[1] memory _publicSignals
    ) public returns (bool) {
        Proof memory proof;
        proof.a = Pairing.G1Point(_pA[0], _pA[1]);
        proof.b = Pairing.G2Point([_pB[0][0], _pB[0][1]], [_pB[1][0], _pB[1][1]]);
        proof.c = Pairing.G1Point(_pC[0], _pC[1]);
        
        uint[] memory inputValues = new uint[](_publicSignals.length);
        for(uint i = 0; i < _publicSignals.length; i++){
            inputValues[i] = _publicSignals[i];
        }
        
        bool isValid = verifyingKey.verifyProof(proof, inputValues);
        
        emit ProofVerified(msg.sender, _publicSignals[0], isValid);
        return isValid;
    }
    
    function verifyHash(
        uint[2] memory _pA,
        uint[2][2] memory _pB,
        uint[2] memory _pC,
        uint256 _hash
    ) external returns (bool) {
        uint[1] memory publicSignals = [_hash];
        return verifyProof(_pA, _pB, _pC, publicSignals);
    }
}

/*
 * Pairing Library for BN254 curve operations
 * Based on the standard pairing library used in zkSNARKs
 */
library Pairing {
    struct G1Point {
        uint X;
        uint Y;
    }
    
    struct G2Point {
        uint[2] X;
        uint[2] Y;
    }
    
    function P1() pure internal returns (G1Point memory) {
        return G1Point(1, 2);
    }
    
    function P2() pure internal returns (G2Point memory) {
        return G2Point(
            [11559732032986387107991004021392285783925812861821192530917403151452391805634,
             10857046999023057135944570762232829481370756359578518086990519993285655852781],
            [4082367875863433681332203403145435568316851327593401208105741076214120093531,
             8495653923123431417604973247489272438418190587263600148770280649306958101930]
        );
    }
    
    function negate(G1Point memory p) pure internal returns (G1Point memory) {
        uint q = 21888242871839275222246405745257275088696311157297823662689037894645226208583;
        if (p.X == 0 && p.Y == 0)
            return G1Point(0, 0);
        return G1Point(p.X, q - (p.Y % q));
    }
    
    function addition(G1Point memory p1, G1Point memory p2) internal view returns (G1Point memory r) {
        uint[4] memory input;
        input[0] = p1.X;
        input[1] = p1.Y;
        input[2] = p2.X;
        input[3] = p2.Y;
        bool success;
        assembly {
            success := staticcall(sub(gas(), 2000), 6, input, 0xc0, r, 0x60)
        }
        require(success, "pairing-add-failed");
    }
    
    function scalar_mul(G1Point memory p, uint s) internal view returns (G1Point memory r) {
        uint[3] memory input;
        input[0] = p.X;
        input[1] = p.Y;
        input[2] = s;
        bool success;
        assembly {
            success := staticcall(sub(gas(), 2000), 7, input, 0x80, r, 0x60)
        }
        require(success, "pairing-mul-failed");
    }
    
    function pairing(G1Point[] memory p1, G2Point[] memory p2) internal view returns (bool) {
        require(p1.length == p2.length, "pairing-lengths-failed");
        uint elements = p1.length;
        uint inputSize = elements * 6;
        uint[] memory input = new uint[](inputSize);
        
        for (uint i = 0; i < elements; i++) {
            input[i * 6 + 0] = p1[i].X;
            input[i * 6 + 1] = p1[i].Y;
            input[i * 6 + 2] = p2[i].X[0];
            input[i * 6 + 3] = p2[i].X[1];
            input[i * 6 + 4] = p2[i].Y[0];
            input[i * 6 + 5] = p2[i].Y[1];
        }
        
        uint[1] memory out;
        bool success;
        assembly {
            success := staticcall(sub(gas(), 2000), 8, add(input, 0x20), mul(inputSize, 0x20), out, 0x20)
        }
        require(success, "pairing-opcode-failed");
        return out[0] != 0;
    }
    
    function verifyProof(
        VerifyingKey memory vk,
        Proof memory proof,
        uint[] memory input
    ) internal view returns (bool) {
        uint256 snark_scalar_field = 21888242871839275222246405745257275088548364400416034343698204186575808495617;
        G1Point memory vk_x = G1Point(0, 0);
        
        for (uint i = 0; i < input.length; i++) {
            require(input[i] < snark_scalar_field, "verifier-gte-snark-scalar-field");
            vk_x = addition(vk_x, scalar_mul(vk.gamma_abc[i + 1], input[i]));
        }
        vk_x = addition(vk_x, vk.gamma_abc[0]);
        
        return pairing(
            [negate(proof.a), vk.alpha, vk_x, proof.c],
            [proof.b, vk.beta, vk.gamma, vk.delta]
        );
    }
}

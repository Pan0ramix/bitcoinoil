// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2022 The Bitcoin Core developers
// Copyright (c) 2022-2023 The Bitcoinoil developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOINOIL_AUXPOW_SERIALIZATION_H
#define BITCOINOIL_AUXPOW_SERIALIZATION_H

#include <primitives/transaction.h>
#include <serialize.h>
#include <uint256.h>

// Forward declaration
class CAuxPow;

/**
 * Serialize an empty CAuxPow object.
 * This is used when a block is flagged as auxpow but doesn't have an actual auxpow.
 */
template<typename Stream>
void SerializeEmptyAuxPow(Stream& s)
{
    // Serialize empty coinbase transaction
    CTransactionRef emptyTx;
    s << emptyTx;
    
    // Serialize empty chain merkle branch
    std::vector<uint256> emptyChainMerkleBranch;
    s << emptyChainMerkleBranch;
    
    // Serialize chain index (0)
    int emptyChainIndex = 0;
    s << emptyChainIndex;
    
    // Serialize empty merkle branch
    std::vector<uint256> emptyMerkleBranch;
    s << emptyMerkleBranch;
    
    // Serialize empty parent block header
    int32_t nVersion = 0;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    uint32_t nTime = 0;
    uint32_t nBits = 0;
    uint32_t nNonce = 0;
    
    s << nVersion;
    s << hashPrevBlock;
    s << hashMerkleRoot;
    s << nTime;
    s << nBits;
    s << nNonce;
}

#endif // BITCOINOIL_AUXPOW_SERIALIZATION_H 
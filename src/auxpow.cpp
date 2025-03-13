// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2022 The Bitcoin Core developers
// Copyright (c) 2022-2023 The Bitcoinoil developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <auxpow.h>
#include <hash.h>
#include <primitives/block.h>
#include <script/script.h>
#include <util/strencodings.h>
#include <algorithm>
#include <consensus/merkle.h>

// Algorithm to check that a parent block merkle root contains a specific
// auxiliary block header hash as one of its merkle leaves.
bool CAuxPow::CheckMerkleBranch(uint256 hash, const std::vector<uint256>& vMerkleBranch, int nIndex)
{
    // The merkle root is stored in reverse byte order
    for (const uint256& node : vMerkleBranch) {
        if (nIndex & 1) {
            hash = Hash(node.begin(), node.end(), hash.begin(), hash.end());
        } else {
            hash = Hash(hash.begin(), hash.end(), node.begin(), node.end());
        }
        nIndex >>= 1;
    }
    
    return nIndex == 0;
}

bool CAuxPow::Check(const uint256& hashAuxBlock, int nChainId)
{
    // Check that the parent merged mining coinbase contains our chain ID
    // This prevents a parent chain from claiming work on multiple auxiliary chains
    
    // Check that the chain merkle root is well-formed
    if (vChainMerkleBranch.size() > 30) {
        return false;
    }
    
    // Check that the coinbase transaction contains the aux block hash as an output
    std::vector<unsigned char> vchRootHash(32);
    std::reverse_copy(hashAuxBlock.begin(), hashAuxBlock.end(), vchRootHash.begin());
    std::vector<unsigned char> vchCoinbaseCommitment;
    
    // The aux hash needs to be in the coinbase scriptSig
    const CScript& script = coinbaseTx->vin[0].scriptSig;
    
    // Check if the coinbase transaction contains the hash of our chain
    bool found = false;
    for (unsigned int i = 0; i + vchRootHash.size() <= script.size(); i++) {
        if (std::equal(script.begin() + i, script.begin() + i + vchRootHash.size(), vchRootHash.begin())) {
            found = true;
            break;
        }
    }
    
    if (!found) {
        return false;
    }
    
    // Check that the block merkle root is well-formed (with max depth 30)
    if (vMerkleBranch.size() > 30) {
        return false;
    }
    
    // Check that the chain ID is correct (parent chain must match its ID)
    if ((parentBlockHeader.nVersion & BLOCK_VERSION_CHAIN_ID_MASK) != 0) {
        return false;
    }
    
    // Current chain ID must match expected chain ID
    const int nAuxPowChainId = (hashAuxBlock.GetUint64(0) & 0xff);
    if (nAuxPowChainId != nChainId) {
        return false;
    }
    
    // The parent block must have the auxpow version bit set
    if (!(parentBlockHeader.nVersion & BLOCK_VERSION_AUXPOW)) {
        return false;
    }
    
    // Verify the merkle branch connecting the coinbase with the aux block header
    uint256 hashTx = coinbaseTx->GetHash();
    if (!CheckMerkleBranch(hashTx, vMerkleBranch, nChainIndex)) {
        return false;
    }
    
    return true;
} 
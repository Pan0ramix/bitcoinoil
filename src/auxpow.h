// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2022 The Bitcoin Core developers
// Copyright (c) 2022-2023 The Bitcoinoil developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOINOIL_AUXPOW_H
#define BITCOINOIL_AUXPOW_H

#include <primitives/transaction.h>
#include <primitives/block.h>
#include <serialize.h>
#include <uint256.h>

class CAuxPow
{
public:
    // Merkle branch linking the aux block to the coinbase transaction
    std::vector<uint256> vChainMerkleBranch;
    // Index of the aux block header in the coinbase
    int nChainIndex;
    
    // Parent coinbase transaction
    CTransactionRef coinbaseTx;
    
    // Parent block header
    CBlockHeader parentBlockHeader;
    
    // Merkle branch linking this block's coinbase to the parent block's merkle root
    std::vector<uint256> vMerkleBranch;
    
    CAuxPow() {}
    
    CAuxPow(CTransactionRef coinbaseTx) : coinbaseTx(coinbaseTx) {}
    
    // Check if the given hash is in the merkle branch
    bool CheckMerkleBranch(uint256 hash, const std::vector<uint256>& vMerkleBranch, int nIndex);
    
    // Verify the AuxPow for this block
    bool Check(const uint256& hashAuxBlock, int nChainId);
    
    SERIALIZE_METHODS(CAuxPow, obj)
    {
        READWRITE(obj.vChainMerkleBranch);
        READWRITE(obj.nChainIndex);
        READWRITE(obj.coinbaseTx);
        READWRITE(obj.parentBlockHeader);
        READWRITE(obj.vMerkleBranch);
    }
};

// Version mask for AuxPow chain ID
static const int BLOCK_VERSION_CHAIN_ID_MASK = 0xF0000000;

// Version bits for AuxPow blocks
static const int BLOCK_VERSION_AUXPOW = (1 << 8);

#endif // BITCOINOIL_AUXPOW_H 
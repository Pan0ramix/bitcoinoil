// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2022 The Bitcoin Core developers
// Copyright (c) 2022-2023 The Bitcoinoil developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "auxpow.h"
#include "auxpow_impl.h"
#include "consensus/merkle.h"
#include <primitives/block.h>
#include <hash.h>
#include <script/script.h>
#include <util/strencodings.h>
#include <algorithm>
#include <auxpow_constants.h>
#include <streams.h>
#include <chainparams.h>
#include <logging.h>
#include <validation.h>
#include <pow.h>
#include <crypto/common.h>

// Default constructor
CAuxPow::CAuxPow() {}

// Constructor with coinbase transaction
CAuxPow::CAuxPow(CTransactionRef coinbaseTx) : coinbaseTx(coinbaseTx) {}

// Destructor implementation
CAuxPow::~CAuxPow() {}

// Explicit instantiation of templates
template void CAuxPow::Serialize<CDataStream>(CDataStream& s) const;
template void CAuxPow::Serialize<CHashWriter>(CHashWriter& s) const;
template void CAuxPow::Unserialize<CDataStream>(CDataStream& s);

// Additional instantiations for DataStream
template void CAuxPow::Serialize<DataStream>(DataStream& s) const;
template void CAuxPow::Unserialize<DataStream>(DataStream& s);

// Implementation of CheckMerkleBranch
uint256 CAuxPow::CheckMerkleBranch(uint256 hash, const std::vector<uint256>& vMerkleBranch, int nIndex) {
    if (nIndex == -1)
        return uint256();
    for (std::vector<uint256>::const_iterator it(vMerkleBranch.begin()); it != vMerkleBranch.end(); ++it) {
        if (nIndex & 1) {
            // Concatenate the hashes and hash them
            std::vector<unsigned char> vchCombined;
            vchCombined.reserve(hash.size() + it->size());
            vchCombined.insert(vchCombined.end(), hash.begin(), hash.end());
            vchCombined.insert(vchCombined.end(), it->begin(), it->end());
            hash = Hash(vchCombined);
        } else {
            // Concatenate the hashes and hash them
            std::vector<unsigned char> vchCombined;
            vchCombined.reserve(it->size() + hash.size());
            vchCombined.insert(vchCombined.end(), it->begin(), it->end());
            vchCombined.insert(vchCombined.end(), hash.begin(), hash.end());
            hash = Hash(vchCombined);
        }
        nIndex >>= 1;
    }
    return hash;
}

// Algorithm to check that a parent block merkle root contains a specific
// auxiliary block hash as the merkle root
bool CAuxPow::Check(const uint256& hashAuxBlock, int nChainId) const {
    if (nChainIndex != 0)
        return error("AuxPow is not a generate");

    if (!coinbaseTx)
        return error("AuxPow does not have coinbase transaction");

    // Check that the chain merkle root is in the coinbase
    const CScript script = coinbaseTx->vin[0].scriptSig;

    // Check that the same work is not submitted twice to our chain.
    const uint256 hashBlock = parentBlockHeader->GetHash();
    if (!CheckProofOfWork(hashBlock, parentBlockHeader->nBits, Params().GetConsensus()))
        return error("AuxPow parent block has invalid proof of work");

    bool fMerkleBranchValid = true;
    uint256 nRootHash = CheckMerkleBranch(hashAuxBlock, vChainMerkleBranch, nChainIndex);
    
    // Verify the merkle branch.
    if (fMerkleBranchValid)
    {
        // This is a bit complicated because we have two merkle branches.
        // The coinbase contains the hash of the block header and the merkle
        // branch, and this needs to be checked against the actual merkle root
        // of the parent block.
        std::vector<uint8_t> vchRootHash(nRootHash.begin(), nRootHash.end());
        std::reverse(vchRootHash.begin(), vchRootHash.end()); // correct endian

        // Check that we are in the parent block merkle tree
        if (CheckMerkleBranch(coinbaseTx->GetHash(), vMerkleBranch, 0) != parentBlockHeader->hashMerkleRoot)
            return error("Aux POW merkle root incorrect");

        // Check that there is at least one input.
        if (coinbaseTx->vin.empty())
            return error("Aux POW coinbase has no inputs");

        const CScript script = coinbaseTx->vin[0].scriptSig;

        // Check that the merkle branch is well-formed.
        if (vchRootHash.size() != 32)
            return error("Aux POW merkle branch size invalid");

        // Check that the script contains the chain ID.
        CScript::const_iterator pc = script.begin();
        opcodetype opcode;
        while (pc < script.end())
        {
            std::vector<uint8_t> vch;
            if (!script.GetOp(pc, opcode, vch))
                break;
            if (vch.size() >= 4)
            {
                int nExpectedChainId = 0;
                for (unsigned int i = 0; i < 4 && i < vch.size(); i++)
                    nExpectedChainId |= (int)vch[vch.size() - 1 - i] << (i * 8);
                if (nExpectedChainId == nChainId)
                    return true;
            }
        }
    }

    return error("Aux POW missing chain ID in parent coinbase");
} 
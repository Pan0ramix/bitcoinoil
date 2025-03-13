// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2022 The Bitcoin Core developers
// Copyright (c) 2022-2023 The Bitcoinoil developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOINOIL_AUXPOW_H
#define BITCOINOIL_AUXPOW_H

#include <consensus/params.h>
#include <primitives/transaction.h>
#include <serialize.h>
#include <uint256.h>

// Forward declaration
class CBlockHeader;

/** Header for merge-mining data structure. */
class CAuxPow
{
private:
    /** The parent block's coinbase transaction.  */
    CTransactionRef coinbaseTx;

    /** The merkle branch connecting the aux block to our coinbase.  */
    std::vector<uint256> vChainMerkleBranch;

    /** The index of the parent chain.  */
    int nChainIndex;

    /** The merkle branch connecting our coinbase to the aux block.  */
    std::vector<uint256> vMerkleBranch;

    /** The parent block header.  */
    std::unique_ptr<CBlockHeader> parentBlockHeader;

public:
    CAuxPow();
    CAuxPow(CTransactionRef coinbaseTx);
    ~CAuxPow();

    // Getter for the parent block header
    const CBlockHeader* GetParentBlockHeader() const {
        return parentBlockHeader.get();
    }

    template<typename Stream>
    void Serialize(Stream& s) const;

    template<typename Stream>
    void Unserialize(Stream& s);

    /**
     * Check a merkle branch.  This used to be in CBlock, but was moved to
     * CAuxPow for the merged mining.
     */
    static uint256 CheckMerkleBranch(uint256 hash,
                                    const std::vector<uint256>& vMerkleBranch,
                                    int nIndex);

    /**
     * Check the auxpow, given the merge-mined block's hash and our chain ID.
     * Note that this does not verify the actual PoW on the parent block, though
     * it does check that the chain ID is embedded in the coinbase transaction.
     */
    bool Check(const uint256& hashAuxBlock, int nChainId) const;
};

#endif // BITCOINOIL_AUXPOW_H 
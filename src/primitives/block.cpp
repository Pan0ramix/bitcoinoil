// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2019 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/block.h>

#include <hash.h>
#include <tinyformat.h>
#include <auxpow.h>
#include <auxpow_impl.h>
#include <streams.h>

// Explicit template instantiations for CBlockHeader serialization
template void Serialize<CHashWriter>(CHashWriter& s, const CBlockHeader& block);

// Explicit template instantiations for CAuxPow serialization
template void CAuxPow::Serialize<CHashWriter>(CHashWriter& s) const;

uint256 CBlockHeader::GetHash() const
{
    // When AuxPow is active and the block has AuxPow data, we need to hash the block
    // without the AuxPow data to maintain compatibility with merge-mined chains
    if (IsAuxPow() && auxpow.get()) {
        // Create a temporary copy of the block header without auxpow to hash
        CBlockHeader blockHeader = *this;
        blockHeader.auxpow.reset(); // Clear auxpow data
        return SerializeHash(blockHeader);
    }
    return SerializeHash(*this);
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}

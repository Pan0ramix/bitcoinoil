// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOINOIL_PRIMITIVES_BLOCK_H
#define BITCOINOIL_PRIMITIVES_BLOCK_H

#include <primitives/transaction.h>
#include <serialize.h>
#include <uint256.h>
#include <util/time.h>
#include <auxpow_constants.h>
#include <auxpow_serialization.h>

// Forward declarations
class CAuxPow;

/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */
class CBlockHeader
{
public:
    // header
    int32_t nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    uint32_t nTime;
    uint32_t nBits;
    uint32_t nNonce;
    
    // Memory only
    mutable std::shared_ptr<CAuxPow> auxpow;

    CBlockHeader()
    {
        SetNull();
    }

    void SetNull()
    {
        nVersion = 0;
        hashPrevBlock.SetNull();
        hashMerkleRoot.SetNull();
        nTime = 0;
        nBits = 0;
        nNonce = 0;
        auxpow.reset();
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

    uint256 GetHash() const;
    
    // Check if this header has auxpow data
    bool IsAuxPow() const
    {
        return (nVersion & BLOCK_VERSION_AUXPOW) != 0;
    }

    NodeSeconds Time() const
    {
        NodeSeconds result(std::chrono::seconds{nTime});
        return result;
    }

    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }
    
    // Add member functions for serialization
    template<typename Stream>
    void Serialize(Stream& s) const {
        ::Serialize(s, *this);
    }

    template<typename Stream>
    void Unserialize(Stream& s) {
        ::Unserialize(s, *this);
    }

    // Get chain ID from version
    int GetChainID() const
    {
        return (nVersion & BLOCK_VERSION_CHAIN_ID_MASK) >> BLOCK_VERSION_CHAIN_ID_SHIFT;
    }
};

// Now include auxpow.h after CBlockHeader is fully defined
#include <auxpow.h>

// Serialization forward declarations
template <typename Stream>
void Serialize(Stream& s, const CBlockHeader& block);

template <typename Stream>
void Unserialize(Stream& s, CBlockHeader& block);

class CBlock : public CBlockHeader
{
public:
    // network and disk
    std::vector<CTransactionRef> vtx;

    // memory only
    mutable bool fChecked;

    CBlock()
    {
        SetNull();
    }

    CBlock(const CBlockHeader &header)
    {
        SetNull();
        *(static_cast<CBlockHeader*>(this)) = header;
    }

    template<typename Stream>
    void Serialize(Stream& s) const
    {
        ::Serialize(s, static_cast<const CBlockHeader&>(*this));
        s << vtx;
    }
    
    template<typename Stream>
    void Unserialize(Stream& s)
    {
        ::Unserialize(s, static_cast<CBlockHeader&>(*this));
        s >> vtx;
    }

    void SetNull()
    {
        CBlockHeader::SetNull();
        vtx.clear();
        fChecked = false;
    }

    CBlockHeader GetBlockHeader() const
    {
        CBlockHeader block;
        block.nVersion       = nVersion;
        block.hashPrevBlock  = hashPrevBlock;
        block.hashMerkleRoot = hashMerkleRoot;
        block.nTime          = nTime;
        block.nBits          = nBits;
        block.nNonce         = nNonce;
        block.auxpow         = auxpow;
        return block;
    }

    std::string ToString() const;
};

/** Describes a place in the block chain to another node such that if the
 * other node doesn't have the same branch, it can find a recent common trunk.
 * The further back it is, the further before the fork it may be.
 */
struct CBlockLocator
{
    std::vector<uint256> vHave;

    CBlockLocator() {}

    explicit CBlockLocator(std::vector<uint256>&& have) : vHave(std::move(have)) {}

    SERIALIZE_METHODS(CBlockLocator, obj)
    {
        int nVersion = s.GetVersion();
        if (!(s.GetType() & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(obj.vHave);
    }

    void SetNull()
    {
        vHave.clear();
    }

    bool IsNull() const
    {
        return vHave.empty();
    }
};

// Implementation for the forward declared serialization functions
template<typename Stream>
void Serialize(Stream& s, const CBlockHeader& block)
{
    s << block.nVersion;
    s << block.hashPrevBlock;
    s << block.hashMerkleRoot;
    s << block.nTime;
    s << block.nBits;
    s << block.nNonce;
    
    // auxpow (optional)
    if (block.IsAuxPow()) {
        if (block.auxpow) {
            s << *block.auxpow;
        } else {
            // If no auxpow is present but the block is flagged as auxpow, 
            // serialize an empty auxpow
            SerializeEmptyAuxPow(s);
        }
    }
}

template<typename Stream>
void Unserialize(Stream& s, CBlockHeader& block)
{
    s >> block.nVersion;
    s >> block.hashPrevBlock;
    s >> block.hashMerkleRoot;
    s >> block.nTime;
    s >> block.nBits;
    s >> block.nNonce;
    
    // auxpow (optional)
    if (block.IsAuxPow()) {
        block.auxpow = std::make_shared<CAuxPow>();
        s >> *block.auxpow;
    }
}

#endif // BITCOINOIL_PRIMITIVES_BLOCK_H

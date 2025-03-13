#ifndef BITCOINOIL_AUXPOW_IMPL_H
#define BITCOINOIL_AUXPOW_IMPL_H

#include <auxpow.h>
#include <primitives/block.h>

// Implementation of CAuxPow template functions
template<typename Stream>
void CAuxPow::Serialize(Stream& s) const {
    s << coinbaseTx;
    s << vChainMerkleBranch;
    s << nChainIndex;
    s << vMerkleBranch;
    
    // Serialize the parent block header
    if (parentBlockHeader) {
        s << *parentBlockHeader;
    } else {
        CBlockHeader emptyHeader;
        s << emptyHeader;
    }
}

template<typename Stream>
void CAuxPow::Unserialize(Stream& s) {
    s >> coinbaseTx;
    s >> vChainMerkleBranch;
    s >> nChainIndex;
    s >> vMerkleBranch;
    
    // Unserialize the parent block header
    parentBlockHeader.reset(new CBlockHeader());
    s >> *parentBlockHeader;
}

#endif // BITCOINOIL_AUXPOW_IMPL_H 
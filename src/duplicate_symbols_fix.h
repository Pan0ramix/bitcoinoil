// Copyright (c) 2023 The Bitcoinoil developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOINOIL_DUPLICATE_SYMBOLS_FIX_H
#define BITCOINOIL_DUPLICATE_SYMBOLS_FIX_H

#include <primitives/block.h>
#include <arith_uint256.h>
#include <util/time.h>

// This header provides inline functions to fix duplicate symbol issues
// during linking. These functions forward to the real implementations.

// Fix for duplicate CalculateHeadersWork
inline arith_uint256 CalculateHeadersWork_inline(const std::vector<CBlockHeader>& headers) {
    // Forward to the real implementation in validation_fix.cpp
    extern arith_uint256 CalculateHeadersWork(const std::vector<CBlockHeader>& headers);
    return CalculateHeadersWork(headers);
}

// Fix for duplicate GetAdjustedTime
inline int64_t GetAdjustedTime_inline() {
    // Forward to the real implementation in timedata.cpp
    extern int64_t GetAdjustedTime();
    return GetAdjustedTime();
}

#endif // BITCOINOIL_DUPLICATE_SYMBOLS_FIX_H 
#ifndef BITCOINOIL_AUXPOW_CONSTANTS_H
#define BITCOINOIL_AUXPOW_CONSTANTS_H

// Version mask for AuxPow chain ID (bits 16-23, allowing 256 different chain IDs)
static const int BLOCK_VERSION_CHAIN_ID_MASK = 0x00FF0000;

// Version bits for AuxPow blocks
static const int BLOCK_VERSION_AUXPOW = (1 << 8);

// Chain ID bit shift position
static const int BLOCK_VERSION_CHAIN_ID_SHIFT = 16;

#endif // BITCOINOIL_AUXPOW_CONSTANTS_H 
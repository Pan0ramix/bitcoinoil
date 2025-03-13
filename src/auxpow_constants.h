#ifndef BITCOINOIL_AUXPOW_CONSTANTS_H
#define BITCOINOIL_AUXPOW_CONSTANTS_H

// Version mask for AuxPow chain ID
static const int BLOCK_VERSION_CHAIN_ID_MASK = 0xF0000000;

// Version bits for AuxPow blocks
static const int BLOCK_VERSION_AUXPOW = (1 << 8);

#endif // BITCOINOIL_AUXPOW_CONSTANTS_H 
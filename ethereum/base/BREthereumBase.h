//
//  BREthereumBase
//  breadwallet-core Ethereum
//
//  Created by Ed Gamble on 3/22/18.
//  Copyright (c) 2018 breadwallet LLC
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.

#ifndef BR_Ethereum_Base_H
#define BR_Ethereum_Base_H

#ifdef __cplusplus
extern "C" {
#endif

#include "BRArray.h"
#include "BRSet.h"

#define BRArrayOf(type)    type*
#define BRSetOf(type)      BRSet*

#define FOR_SET(type,var,set) \
  for (type var = BRSetIterate(set, NULL); \
       NULL != var; \
       var = BRSetIterate(set, var))

typedef void
(*BRSetItemFree) (void *item);

static inline void
BRSetFreeAll (BRSet *set, BRSetItemFree itemFree) {
    size_t itemsCount = BRSetCount (set);
    void  *itemsAll [itemsCount];

    BRSetAll (set, itemsAll,  itemsCount);
    for (size_t index = 0; index < itemsCount; index++)
        itemFree (itemsAll[index]);
    BRSetClear (set);
    BRSetFree  (set);
}

#include "../util/BRUtil.h"         // "BRInt.h"
#include "../rlp/BRRlp.h"
#include "BREthereumLogic.h"
#include "BREthereumEther.h"
#include "BREthereumGas.h"
#include "BREthereumHash.h"
#include "BREthereumData.h"
#include "BREthereumAddress.h"
#include "BREthereumSignature.h"

typedef enum {
    RLP_TYPE_NETWORK,
    RLP_TYPE_ARCHIVE,
    RLP_TYPE_TRANSACTION_UNSIGNED,
    RLP_TYPE_TRANSACTION_SIGNED = RLP_TYPE_NETWORK,
} BREthereumRlpType;


//
// Sync Mode
//
typedef enum {
    //
    // We'll be willing to do a complete block chain sync, even starting at block zero.  We'll
    // use our 'N-ary Search on Account Changes' to perform the sync effectively.  We'll use the
    // BRD endpoint to augment the 'N-Ary Search' to find TOK transfers where our address is the
    // SOURCE.
    //
    SYNC_MODE_FULL_BLOCKCHAIN,
    
    //
    // We'll use the BRD endpoint to identiy blocks of interest based on ETH and TOK transfer
    // where our addres is the SOURCE or TARGET. We'll only process blocks from the last N (~ 2000)
    // blocks in the chain.
    //
    SYNC_MODE_PRIME_WITH_ENDPOINT
} BREthereumSyncMode;

//
// Sync Interest
//
typedef enum {
    CLIENT_GET_BLOCKS_NODE = 0,
    CLIENT_GET_BLOCKS_TRANSACTIONS_AS_SOURCE = (1 << 0),
    CLIENT_GET_BLOCKS_TRANSACTIONS_AS_TARGET = (1 << 1),
    CLIENT_GET_BLOCKS_LOGS_AS_SOURCE = (1 << 2),
    CLIENT_GET_BLOCKS_LOGS_AS_TARGET = (1 << 3)
} BREthereumSyncInterest;

//
// Sync Interest Set
//
typedef unsigned int BREthereumSyncInterestSet;

static inline int // 1 if match; 0 if not
syncInterestMatch(BREthereumSyncInterestSet interests,
                  BREthereumSyncInterest interest) {
    return interests & interest;
}

#ifdef __cplusplus
}
#endif

#endif /* BR_Ethereum_Base_H */

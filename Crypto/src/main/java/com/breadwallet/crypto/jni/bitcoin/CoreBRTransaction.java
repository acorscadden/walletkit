package com.breadwallet.crypto.jni.bitcoin;

import com.breadwallet.crypto.jni.CryptoLibrary;
import com.breadwallet.crypto.jni.SizeT;
import com.breadwallet.crypto.jni.support.UInt256;
import com.google.common.base.Optional;

public interface CoreBRTransaction {

    static Optional<CoreBRTransaction> create(byte[] rawData, int timestamp, int blockHeight) {
        BRTransaction transaction = CryptoLibrary.INSTANCE.BRTransactionParse(rawData, new SizeT(rawData.length));
        if (null != transaction) {
            transaction.timestamp = timestamp;
            transaction.blockHeight = blockHeight;
        }
        return Optional.fromNullable(transaction).transform(OwnedBRTransaction::new);
    }

    static Optional<CoreBRTransaction> create(BRWallet wallet, long amount, String address) {
        BRTransaction transaction = CryptoLibrary.INSTANCE.BRWalletCreateTransaction(wallet, amount, address);
        return Optional.fromNullable(transaction).transform(OwnedBRTransaction::new);
    }

    BRTxInput[] getInputs();

    BRTxOutput[] getOutputs();

    UInt256 getTxHash();

    BRTransaction asBRTransaction();

    BRTransaction asBRTransactionDeepCopy();

    byte[] serialize();
}

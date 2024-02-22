// Copyright (c) 2011-2017 The Cryptonote developers
// Copyright (c) 2017-2018 The Circle Foundation & Conceal Devs
// Copyright (c) 2018-2023 Conceal Network & Conceal Devs
//
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <limits>
#include <string>
#include <system_error>
#include <vector>
#include <boost/optional.hpp>
#include "CryptoNote.h"
#include "CryptoNoteConfig.h"
#include "IObservable.h"

namespace cn
{

using DepositId = size_t;
using TransactionId = size_t;

const size_t WALLET_INVALID_TRANSACTION_ID = std::numeric_limits<size_t>::max();
const size_t WALLET_INVALID_TRANSFER_ID = std::numeric_limits<size_t>::max();
const size_t WALLET_INVALID_DEPOSIT_ID = std::numeric_limits<size_t>::max();
const uint32_t WALLET_UNCONFIRMED_TRANSACTION_HEIGHT = std::numeric_limits<uint32_t>::max();

enum class WalletTransactionState : uint8_t
{
  SUCCEEDED = 0,
  FAILED,
  CANCELLED,
  CREATED,
  DELETED
};

enum class WalletSaveLevel : uint8_t {
  SAVE_KEYS_ONLY,
  SAVE_KEYS_AND_TRANSACTIONS,
  SAVE_ALL
};

enum WalletEventType
{
  TRANSACTION_CREATED,
  TRANSACTION_UPDATED,
  BALANCE_UNLOCKED,
  SYNC_PROGRESS_UPDATED,
  SYNC_COMPLETED,
};

struct WalletTransactionCreatedData
{
  size_t transactionIndex;
};

struct Deposit
{
  size_t creatingTransactionId;
  size_t spendingTransactionId;
  uint32_t term;
  uint64_t amount;
  uint64_t interest;
  uint64_t height;
  uint64_t unlockHeight;
  bool locked;
  uint32_t outputInTransaction;
  crypto::Hash transactionHash;
  std::string address;
};

struct WalletTransactionUpdatedData
{
  size_t transactionIndex;
};

struct WalletSynchronizationProgressUpdated
{
  uint32_t processedBlockCount;
  uint32_t totalBlockCount;
};

struct WalletEvent
{
  WalletEventType type;
  union {
    WalletTransactionCreatedData transactionCreated;
    WalletTransactionUpdatedData transactionUpdated;
    WalletSynchronizationProgressUpdated synchronizationProgressUpdated;
  };
};

struct WalletTransaction
{
  WalletTransactionState state;
  uint64_t timestamp;
  uint32_t blockHeight;
  crypto::Hash hash;
  boost::optional<crypto::SecretKey> secretKey;
  int64_t totalAmount;
  uint64_t fee;
  uint64_t creationTime;
  uint64_t unlockTime;
  std::string extra;
  size_t firstDepositId = WALLET_INVALID_DEPOSIT_ID;
  size_t depositCount = 0;
  bool isBase;
  std::vector<std::string> messages;
};

enum class WalletTransferType : uint8_t
{
  USUAL = 0,
  DONATION,
  CHANGE
};

struct WalletOrder
{
  std::string address;
  uint64_t amount;
};

struct WalletMessage
{
  std::string address;
  std::string message;
};

struct WalletTransfer
{
  WalletTransferType type;
  std::string address;
  int64_t amount;
};

struct DonationSettings
{
  std::string address;
  uint64_t threshold = 0;
};

struct TransactionParameters
{
  std::vector<std::string> sourceAddresses;
  std::vector<WalletOrder> destinations;
  std::vector<WalletMessage> messages;
  uint64_t fee = cn::parameters::MINIMUM_FEE_V2;
  uint64_t mixIn = cn::parameters::MINIMUM_MIXIN;
  std::string extra;
  DepositId firstDepositId = WALLET_INVALID_DEPOSIT_ID;
  size_t depositCount = 0;
  uint64_t unlockTimestamp = 0;
  DonationSettings donation;
  std::string changeDestination;
  uint64_t ttl = 0;
};

struct WalletTransactionWithTransfers
{
  WalletTransaction transaction;
  std::vector<WalletTransfer> transfers;
};

struct TransactionsInBlockInfo
{
  crypto::Hash blockHash;
  std::vector<WalletTransactionWithTransfers> transactions;
};

struct DepositsInBlockInfo
{
  crypto::Hash blockHash;
  std::vector<Deposit> deposits;
};

struct PaymentIdTransactions
{
  crypto::Hash paymentId;
  std::vector<WalletTransaction> transactions;
};

class TransactionOutputInformation;
class IBlockchainSynchronizerObserver;

class IWalletObserver
{
public:
  virtual ~IWalletObserver() = default;

  virtual void initCompleted(std::error_code result) = 0;
  virtual void saveCompleted(std::error_code result) = 0;
  virtual void synchronizationProgressUpdated(uint32_t current, uint32_t total) = 0;
  virtual void synchronizationCompleted(std::error_code result) = 0;
  virtual void actualBalanceUpdated(uint64_t balance) = 0;
  virtual void pendingBalanceUpdated(uint64_t balance) = 0;
  virtual void actualDepositBalanceUpdated(uint64_t balance) = 0;
  virtual void pendingDepositBalanceUpdated(uint64_t balance) = 0;
  virtual void actualInvestmentBalanceUpdated(uint64_t balance) = 0;
  virtual void pendingInvestmentBalanceUpdated(uint64_t balance) = 0;
  virtual void externalTransactionCreated(TransactionId transactionId) = 0;
  virtual void sendTransactionCompleted(TransactionId transactionId, std::error_code result) = 0;
  virtual void transactionUpdated(TransactionId transactionId) = 0;
  virtual void depositUpdated(DepositId depositId) = 0;
  virtual void depositsUpdated(const std::vector<DepositId> &depositIds) = 0;
};

class IWallet : public IObservable<IWalletObserver>
{
public:
  virtual ~IWallet() = default;

  virtual void initialize(const std::string& path, const std::string& password) = 0;
  virtual void createDeposit(uint64_t amount, uint32_t term, std::string sourceAddress, std::string destinationAddress, std::string &transactionHash) = 0;
  virtual void withdrawDeposit(DepositId depositId, std::string &transactionHash) = 0;
  virtual Deposit getDeposit(size_t depositIndex) const = 0;
  virtual void initializeWithViewKey(const std::string& path, const std::string& password, const crypto::SecretKey& viewSecretKey) = 0;
  virtual void generateNewWallet(const std::string &path, const std::string &password) = 0;
  virtual void load(const std::string& path, const std::string& password, std::string& extra) = 0;
  virtual void load(const std::string& path, const std::string& password) = 0;
  virtual void shutdown() = 0;
  virtual void reset(const uint64_t scanHeight) = 0;
  virtual void exportWallet(const std::string &path, WalletSaveLevel saveLevel, bool encrypt = true, const std::string &extra = "") = 0;

  virtual void changePassword(const std::string &oldPassword, const std::string &newPassword) = 0;
  virtual void save(WalletSaveLevel saveLevel = WalletSaveLevel::SAVE_ALL, const std::string& extra = "") = 0;

  virtual size_t getAddressCount() const = 0;

  virtual size_t getWalletDepositCount() const = 0;  
  virtual std::vector<DepositsInBlockInfo> getDeposits(const crypto::Hash &blockHash, size_t count) const = 0;
  virtual std::vector<DepositsInBlockInfo> getDeposits(uint32_t blockIndex, size_t count) const = 0;

  virtual std::string getAddress(size_t index) const = 0;
  virtual KeyPair getAddressSpendKey(size_t index) const = 0;
  virtual KeyPair getAddressSpendKey(const std::string &address) const = 0;
  virtual KeyPair getViewKey() const = 0;
  virtual std::string createAddress() = 0;
  virtual std::string createAddress(const crypto::SecretKey &spendSecretKey) = 0;
  virtual std::string createAddress(const crypto::PublicKey &spendPublicKey) = 0;
  virtual std::vector<std::string> createAddressList(const std::vector<crypto::SecretKey> &spendSecretKeys, bool reset = true) = 0;
  virtual void deleteAddress(const std::string &address) = 0;

  virtual uint64_t getActualBalance() const = 0;
  virtual uint64_t getActualBalance(const std::string &address) const = 0;
  virtual uint64_t getPendingBalance() const = 0;
  virtual uint64_t getPendingBalance(const std::string &address) const = 0;
  virtual uint64_t getDustBalance() const = 0;
  virtual uint64_t getDustBalance(const std::string &address) const = 0;

  virtual uint64_t getLockedDepositBalance() const = 0;
  virtual uint64_t getLockedDepositBalance(const std::string &address) const = 0;
  virtual uint64_t getUnlockedDepositBalance() const = 0;
  virtual uint64_t getUnlockedDepositBalance(const std::string &address) const = 0;

  virtual size_t getTransactionCount() const = 0;
  virtual WalletTransaction getTransaction(size_t transactionIndex) const = 0;
  virtual size_t getTransactionTransferCount(size_t transactionIndex) const = 0;
  virtual WalletTransfer getTransactionTransfer(size_t transactionIndex, size_t transferIndex) const = 0;

  virtual WalletTransactionWithTransfers getTransaction(const crypto::Hash &transactionHash) const = 0;

  virtual std::vector<TransactionsInBlockInfo> getTransactions(const crypto::Hash &blockHash, size_t count) const = 0;
  virtual std::vector<TransactionsInBlockInfo> getTransactions(uint32_t blockIndex, size_t count) const = 0;



  virtual std::vector<crypto::Hash> getBlockHashes(uint32_t blockIndex, size_t count) const = 0;
  virtual uint32_t getBlockCount() const = 0;
  virtual std::vector<WalletTransactionWithTransfers> getUnconfirmedTransactions() const = 0;
  virtual std::vector<size_t> getDelayedTransactionIds() const = 0;

  virtual size_t transfer(const TransactionParameters &sendingTransaction, crypto::SecretKey &transactionSK) = 0;

  virtual size_t makeTransaction(const TransactionParameters &sendingTransaction) = 0;
  virtual void commitTransaction(size_t transactionId) = 0;
  virtual void rollbackUncommitedTransaction(size_t transactionId) = 0;

  virtual std::vector<TransactionOutputInformation> getUnspentOutputs() = 0;
  virtual size_t getUnspentOutputsCount() = 0;
  virtual std::string getReserveProof(const std::string &address, const uint64_t &reserve, const std::string &message) = 0;
  virtual bool getTxProof(const crypto::Hash &transactionHash, const cn::AccountPublicAddress &address, const crypto::SecretKey &tx_key, std::string &signature) = 0;
  virtual crypto::SecretKey getTransactionDeterministicSecretKey(crypto::Hash &transactionHash) const = 0;
  virtual size_t createOptimizationTransaction(const std::string &address) = 0;
  virtual std::vector<PaymentIdTransactions> getTransactionsByPaymentIds(const std::vector<crypto::Hash> &paymentIds) = 0;

  virtual void start() = 0;
  virtual void stop() = 0;

  //blocks until an event occurred
  virtual WalletEvent getEvent() = 0;
};

} // namespace cn

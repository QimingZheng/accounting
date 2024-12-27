#pragma once

/** An accounting app. */
#include <assert.h>

#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <iomanip>

typedef float real_t;
typedef int64_t uuid_t;
typedef uuid_t proof_id_t;
typedef uuid_t account_id_t;

struct Datetime {
  int year;
  int month;
  int day;
};

class UniqueObject {
 public:
  UniqueObject();

  uuid_t GetId();

 private:
  static uuid_t global_uid_next_;
  uuid_t id_;
};

class Account;

class ObjectsStore {
 public:
  static ObjectsStore* GetObjectsStoreInstance() {
    if (instance_ == nullptr) {
      instance_ = new ObjectsStore();
    }
    return instance_;
  }

  template <typename T>
  T* Get(uuid_t id) {
    if (objects_.find(id) == objects_.end()) {
      return nullptr;
    }
    return static_cast<T*>(objects_.find(id)->second);
  }

  void Add(uuid_t id, UniqueObject*);
  void AddMetaAccount(account_id_t id);

  std::unordered_set<account_id_t> GetMetaAccounts();

 private:
  static ObjectsStore* instance_;
  std::unordered_map<uuid_t, UniqueObject*> objects_;
  std::unordered_set<account_id_t> meta_accounts_;
};

// A proof can be receipt, invoice, contract etc.
class Proof : public UniqueObject {
 public:
 private:
};

class JournalEntry : public UniqueObject {
 private:
  JournalEntry(Datetime datetime, std::vector<proof_id_t> proofs,
               std::vector<std::pair<account_id_t, real_t>> debits,
               std::vector<std::pair<account_id_t, real_t>> credits)
      : datetime_(datetime),
        proofs_(proofs),
        debits_(debits),
        credits_(credits) {}

 public:
  static JournalEntry* CreateJournalEntry(Datetime datetime,
                                          std::vector<proof_id_t> proofs,
                                          account_id_t debit_account,
                                          real_t debit_amount,
                                          account_id_t credit_account,
                                          real_t credit_amount);

  void Accounts();

 private:
  Datetime datetime_;
  std::vector<proof_id_t> proofs_;
  std::vector<std::pair<account_id_t, real_t>> debits_;
  std::vector<std::pair<account_id_t, real_t>> credits_;
};

class Account : public UniqueObject {
 protected:
  Account(std::string name) : name_(name) {}

 public:
  void Credit(Datetime datetime, real_t amount);
  void Debit(Datetime datetime, real_t amount);
  bool Credit(account_id_t account, Datetime datetime, real_t amount);
  bool Debit(account_id_t account, Datetime datetime, real_t amount);

  real_t Balance() const;
  std::string FormatAsTAccount();

 private:
  // Returns:
  //    1. -1 if non of its child-accounts is containing `account`.
  //    2. 0 if this account is `account`.
  account_id_t GetParentAccountOf(account_id_t account);
  std::string name_;

  std::vector<account_id_t> child_accounts_;

  std::vector<std::pair<Datetime, real_t>> debits_;
  std::vector<std::pair<Datetime, real_t>> credits_;
};

class Asset : public Account {
 public:
  Asset();
};
class Liabilities : public Account {
 public:
  Liabilities();
};
class Revenue : public Account {
 public:
  Revenue();
};
class Expense : public Account {
 public:
  Expense();
};
class Equity : public Account {
 public:
  Equity();
};

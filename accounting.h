#pragma once

/** An accounting app. */
#include <assert.h>

#include <algorithm>
#include <iostream>
#include <vector>

typedef float real_t;
typedef int64_t uuid_t;
typedef uuid_t proof_id_t;
typedef uuid_t account_id_t;

class UniqueObject {
 public:
  UniqueObject();

  uuid_t GetId();

 private:
  static uuid_t global_uid_next_;
  uuid_t id_;
};

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
    if (objects_.find(id) != objects_.end()) {
      return nullptr;
    }
    return static_cast<T*>(objects_.find(id)->second);
  }

  void Add(uuid_t id, UniqueObject*);

 private:
  static ObjectsStore* instance_;
  std::unordered_map<uuid_t, UniqueObject*> objects_;
};

// A proof can be receipt, invoice, contract etc.
class Proof : public UniqueObject {
 public:
 private:
};

class JournalEntry : public UniqueObject {
 private:
  JournalEntry();

 public:
  JournalEntry* CreateJournalEntry();

 private:
  std::vector<proof_id_t> proofs_;
  std::vector<std::pair<account_id_t, real_t>> debits_;
  std::vector<std::pair<account_id_t, real_t>> credits_;
};

class Account : public UniqueObject {
 protected:
  Account(std::string name) : name_(name) {}

 public:
  Account* CreateAccount(std::string name);

  void Credit(real_t amount);
  void Debit(real_t amount);
  void Credit(account_id_t account, real_t amount);
  void Debit(account_id_t account, real_t amount);

 private:
  // Returns:
  //    1. -1 if non of its child-accounts is containing `account`.
  //    2. 0 if this account is `account`.
  account_id_t GetParentAccountOf(account_id_t account);
  std::string name_;

  std::vector<account_id_t> child_accounts_;

  std::vector<real_t> debits_;
  std::vector<real_t> credits_;
};

class Asset : public Account {
 public:
  Asset() : Account("Asset") {}
};

class Liabilities : public Account {
 public:
  Liabilities() : Account("Liabilities") {}
};

class Revenue : public Account {
 public:
  Revenue() : Account("Expense") {}
};

class Expense : public Account {
 public:
  Expense() : Account("Expense") {}
};

class Equity : public Account {
 public:
  Equity() : Account("Equity") {}
};

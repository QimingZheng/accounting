#include <accounting.h>

uuid_t UniqueObject::global_uid_next_ = 1;
ObjectsStore* ObjectsStore::instance_ = nullptr;

UniqueObject::UniqueObject() {
  id_ = global_uid_next_++;
  ObjectsStore::GetObjectsStoreInstance()->Add(id_, this);
}

uuid_t UniqueObject::GetId() { return id_; }

void ObjectsStore::Add(uuid_t id, UniqueObject* obj) {
  assert(obj->GetId() == id);
  assert(objects_.find(id) == objects_.end());
  objects_.insert({id, obj});
}

std::unordered_set<account_id_t> ObjectsStore::GetMetaAccounts() {
  return meta_accounts_;
}
void ObjectsStore::AddMetaAccount(account_id_t id) {
  assert(objects_.find(id) != objects_.end());
  assert(meta_accounts_.find(id) == meta_accounts_.end());
  meta_accounts_.insert(id);
}

account_id_t Account::GetParentAccountOf(account_id_t account) {
  if (GetId() == account) {
    return 0;
  }
  for (auto child_account : child_accounts_) {
    account_id_t parent_account = ObjectsStore::GetObjectsStoreInstance()
                                      ->Get<Account>(child_account)
                                      ->GetParentAccountOf(account);
    if (parent_account >= 0) {
      return child_account;
    }
  }
  return -1;
}

void Account::Credit(real_t amount) { credits_.push_back(amount); }
void Account::Debit(real_t amount) { debits_.push_back(amount); }
bool Account::Credit(account_id_t account, real_t amount) {
  account_id_t parent_account_of = GetParentAccountOf(account);
  if (parent_account_of < 0) return false;
  if (parent_account_of > 0) {
    ObjectsStore::GetObjectsStoreInstance()
        ->Get<Account>(parent_account_of)
        ->Credit(account, amount);
  }
  Credit(amount);
  return true;
}
bool Account::Debit(account_id_t account, real_t amount) {
  account_id_t parent_account_of = GetParentAccountOf(account);
  if (parent_account_of < 0) return false;
  if (parent_account_of > 0) {
    ObjectsStore::GetObjectsStoreInstance()
        ->Get<Account>(parent_account_of)
        ->Debit(account, amount);
  }
  Debit(amount);
  return true;
}
real_t Account::Balance() const {
  real_t debits = 0, credits = 0;
  for (auto d : debits_) {
    debits += d;
  }
  for (auto c : credits_) {
    credits += c;
  }
  return debits - credits;
}

JournalEntry* JournalEntry::CreateJournalEntry(std::vector<proof_id_t> proofs,
                                               account_id_t debit_account,
                                               real_t debit_amount,
                                               account_id_t credit_account,
                                               real_t credit_amount) {
  return new JournalEntry(proofs, {{debit_account, debit_amount}},
                          {{credit_account, credit_amount}});
}

void JournalEntry::Accounts() {
  for (auto debit : debits_) {
    for (auto acc :
         ObjectsStore::GetObjectsStoreInstance()->GetMetaAccounts()) {
      Account* meta_acc =
          ObjectsStore::GetObjectsStoreInstance()->Get<Account>(acc);
      assert(meta_acc != nullptr);
      if (meta_acc->Debit(debit.first, debit.second)) {
        break;
      }
    }
  }
  for (auto credit : credits_) {
    for (auto acc :
         ObjectsStore::GetObjectsStoreInstance()->GetMetaAccounts()) {
      if (ObjectsStore::GetObjectsStoreInstance()->Get<Account>(acc)->Credit(
              credit.first, credit.second)) {
        break;
      }
    }
  }
}

Asset::Asset() : Account("Asset") {
  ObjectsStore::GetObjectsStoreInstance()->AddMetaAccount(GetId());
}
Liabilities::Liabilities() : Account("Liabilities") {
  ObjectsStore::GetObjectsStoreInstance()->AddMetaAccount(GetId());
}
Revenue::Revenue() : Account("Revenue") {
  ObjectsStore::GetObjectsStoreInstance()->AddMetaAccount(GetId());
}
Expense::Expense() : Account("Expense") {
  ObjectsStore::GetObjectsStoreInstance()->AddMetaAccount(GetId());
}
Equity::Equity() : Account("Equity") {
  ObjectsStore::GetObjectsStoreInstance()->AddMetaAccount(GetId());
}

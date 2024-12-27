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

Account* Account::CreateAccount(std::string name) { return new Account(name); }

account_id_t Account::GetParentAccountOf(account_id_t account) {
  if (GetId() == account) {
    return 0;
  }
  for (auto child_account : child_accounts_) {
    account_id_t parent_account = ObjectsStore::GetObjectsStoreInstance()
                                      ->Get<Account>(child_account)
                                      ->GetParentAccountOf(account);
    if (parent_account > 0) {
      return child_account;
    }
  }
  return -1;
}

void Account::Credit(real_t amount) { credits_.push_back(amount); }
void Account::Debit(real_t amount) { debits_.push_back(amount); }
void Account::Credit(account_id_t account, real_t amount) {
  account_id_t parent_account_of = GetParentAccountOf(account);
  assert(parent_account_of >= 0);
  if (parent_account_of > 0) {
    ObjectsStore::GetObjectsStoreInstance()
        ->Get<Account>(parent_account_of)
        ->Credit(account, amount);
  }
  Credit(amount);
}
void Account::Debit(account_id_t account, real_t amount) {
  account_id_t parent_account_of = GetParentAccountOf(account);
  assert(parent_account_of >= 0);
  if (parent_account_of > 0) {
    ObjectsStore::GetObjectsStoreInstance()
        ->Get<Account>(parent_account_of)
        ->Debit(account, amount);
  }
  Debit(amount);
}

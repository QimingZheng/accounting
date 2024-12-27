#include <accounting.h>

bool operator<(const Datetime& a, const Datetime& b) {
  if (a.year != b.year) return a.year < b.year;
  if (a.month != b.month) return a.month < b.month;
  if (a.day != b.day) return a.day < b.day;
  return true;
}
bool operator==(const Datetime& a, const Datetime& b) {
  return a.year == b.year && a.month == b.month && a.day == b.day;
}

// -------- UniqueObject --------
uuid_t UniqueObject::global_uid_next_ = 1;
UniqueObject::UniqueObject() {
  id_ = global_uid_next_++;
  ObjectsStore::GetObjectsStoreInstance()->Add(id_, this);
}
uuid_t UniqueObject::GetId() { return id_; }

// -------- ObjectsStore --------
ObjectsStore* ObjectsStore::instance_ = nullptr;
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

// -------- Account --------
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
void Account::Credit(Datetime datetime, real_t amount) {
  credits_.push_back({datetime, amount});
}
void Account::Debit(Datetime datetime, real_t amount) {
  debits_.push_back({datetime, amount});
}
bool Account::Credit(account_id_t account, Datetime datetime, real_t amount) {
  account_id_t parent_account_of = GetParentAccountOf(account);
  if (parent_account_of < 0) return false;
  if (parent_account_of > 0) {
    ObjectsStore::GetObjectsStoreInstance()
        ->Get<Account>(parent_account_of)
        ->Credit(account, datetime, amount);
  }
  Credit(datetime, amount);
  return true;
}
bool Account::Debit(account_id_t account, Datetime datetime, real_t amount) {
  account_id_t parent_account_of = GetParentAccountOf(account);
  if (parent_account_of < 0) return false;
  if (parent_account_of > 0) {
    ObjectsStore::GetObjectsStoreInstance()
        ->Get<Account>(parent_account_of)
        ->Debit(account, datetime, amount);
  }
  Debit(datetime, amount);
  return true;
}
real_t Account::Balance() const {
  real_t debits = 0, credits = 0;
  for (auto d : debits_) {
    debits += d.second;
  }
  for (auto c : credits_) {
    credits += c.second;
  }
  return debits - credits;
}
std::string Account::FormatAsTAccount() {
  std::stringstream formating_stream;
  formating_stream << "\n---------------------\n";
  formating_stream << name_;
  formating_stream << "\n---------------------\n";
  sort(debits_.begin(), debits_.end(),
       [&](const std::pair<Datetime, real_t>& a,
           const std::pair<Datetime, real_t>& b) { return a.first < b.first; });
  sort(credits_.begin(), credits_.end(),
       [](const std::pair<Datetime, real_t>& a,
          const std::pair<Datetime, real_t>& b) { return a.first < b.first; });
  int i = 0, j = 0;
  auto print_real = [&](real_t x) {
    formating_stream << std::fixed << std::setprecision(2) << std::setw(8) << x;
  };
  while (i < debits_.size() && j < credits_.size()) {
    if (debits_[i].first == credits_[j].first) {
      print_real(debits_[i].second);
      formating_stream << " | ";
      print_real(credits_[j].second);
      formating_stream << "\n";
      i++;
      j++;
    } else if (debits_[i].first < credits_[j].first) {
      print_real(debits_[i].second);
      formating_stream << " |\n";
      i++;
    } else {
      formating_stream << "          | ";
      print_real(credits_[j].second);
      formating_stream << "\n";
      j++;
    }
  }
  while (i < debits_.size()) {
    print_real(debits_[i].second);
    formating_stream << " |\n";
    i++;
  }
  while (j < credits_.size()) {
    formating_stream << "          | ";
    print_real(credits_[j].second);
    formating_stream << "\n";
    j++;
  }
  if (i == 0 && j == 0) {
    formating_stream << "          | ";
  }
  return formating_stream.str();
}

// -------- JournalEntry --------
JournalEntry* JournalEntry::CreateJournalEntry(Datetime datetime,
                                               std::vector<proof_id_t> proofs,
                                               account_id_t debit_account,
                                               real_t debit_amount,
                                               account_id_t credit_account,
                                               real_t credit_amount) {
  return new JournalEntry(datetime, proofs, {{debit_account, debit_amount}},
                          {{credit_account, credit_amount}});
}
void JournalEntry::Accounts() {
  for (auto debit : debits_) {
    for (auto acc :
         ObjectsStore::GetObjectsStoreInstance()->GetMetaAccounts()) {
      Account* meta_acc =
          ObjectsStore::GetObjectsStoreInstance()->Get<Account>(acc);
      assert(meta_acc != nullptr);
      if (meta_acc->Debit(debit.first, datetime_, debit.second)) {
        break;
      }
    }
  }
  for (auto credit : credits_) {
    for (auto acc :
         ObjectsStore::GetObjectsStoreInstance()->GetMetaAccounts()) {
      if (ObjectsStore::GetObjectsStoreInstance()->Get<Account>(acc)->Credit(
              credit.first, datetime_, credit.second)) {
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

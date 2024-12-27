#include <accounting.h>

int main() {
  Asset asset;
  Liabilities liabilities;
  JournalEntry* entry = JournalEntry::CreateJournalEntry(
      {}, asset.GetId(), 1000, liabilities.GetId(), 1000);
  entry->Accounts();
  delete entry;
  std::cout << "Asset: " << asset.Balance() << "\n";
  std::cout << "Liabilities: " << liabilities.Balance() << "\n";
  return 0;
}
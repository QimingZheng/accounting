#include <accounting.h>

int main() {
  Asset asset;
  Liabilities liabilities;
  Equity equity;
  JournalEntry* first_entry = JournalEntry::CreateJournalEntry(
      {2024, 12, 27}, {}, asset.GetId(), 1000, liabilities.GetId(), -1000);
  first_entry->Accounts();
  delete first_entry;
  JournalEntry* second_entry = JournalEntry::CreateJournalEntry(
      {2024, 12, 28}, {}, asset.GetId(), -1000, liabilities.GetId(), 1000);
  second_entry->Accounts();
  delete second_entry;
  JournalEntry* third_entry = JournalEntry::CreateJournalEntry(
      {2024, 12, 28}, {}, asset.GetId(), 100, liabilities.GetId(), 100);
  third_entry->Accounts();
  delete third_entry;
  std::cout << "Asset: " << asset.Balance() << "\n";
  std::cout << asset.FormatAsTAccount() << "\n";
  std::cout << "Liabilities: " << liabilities.Balance() << "\n";
  std::cout << liabilities.FormatAsTAccount() << "\n";
  std::cout << equity.FormatAsTAccount() << "\n";
  return 0;
}
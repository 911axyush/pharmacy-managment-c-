/*
 * ============================================================
 *  PHARMACY INVENTORY & EXPIRY MANAGEMENT SYSTEM
 *  Case Study | C++ Implementation
 * ============================================================
 */

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <limits>

using namespace std;

// ─────────────────────────────────────────────
//  Date Structure
// ─────────────────────────────────────────────
struct Date {
    int day, month, year;

    Date(int d = 1, int m = 1, int y = 2024) : day(d), month(m), year(y) {}

    string toString() const {
        ostringstream oss;
        oss << setfill('0') << setw(2) << day << "/"
            << setw(2) << month << "/" << year;
        return oss.str();
    }

    bool operator<(const Date& other) const {
        if (year != other.year) return year < other.year;
        if (month != other.month) return month < other.month;
        return day < other.day;
    }

    bool operator==(const Date& other) const {
        return day == other.day && month == other.month && year == other.year;
    }

    // Days difference: (this - other). Negative means this is earlier.
    int daysFrom(const Date& other) const {
        auto toJulian = [](int d, int m, int y) -> long {
            int a = (14 - m) / 12;
            int yy = y + 4800 - a;
            int mm = m + 12 * a - 3;
            return d + (153 * mm + 2) / 5 + 365L * yy + yy / 4 - yy / 100 + yy / 400 - 32045;
        };
        return (int)(toJulian(day, month, year) - toJulian(other.day, other.month, other.year));
    }
};

// ─────────────────────────────────────────────
//  Medicine Class
// ─────────────────────────────────────────────
class Medicine {
private:
    static int idCounter;
    int id;
    string name;
    string category;       // e.g. Antibiotic, Analgesic, Vitamin
    string manufacturer;
    Date   manufactureDate;
    Date   expiryDate;
    int    quantity;       // units in stock
    double pricePerUnit;
    int    reorderLevel;   // alert when stock falls below this

public:
    Medicine(string n, string cat, string mfr,
             Date mfgDate, Date expDate,
             int qty, double price, int reorder)
        : id(++idCounter), name(n), category(cat), manufacturer(mfr),
          manufactureDate(mfgDate), expiryDate(expDate),
          quantity(qty), pricePerUnit(price), reorderLevel(reorder) {}

    // Getters
    int         getId()           const { return id; }
    string      getName()         const { return name; }
    string      getCategory()     const { return category; }
    string      getManufacturer() const { return manufacturer; }
    Date        getExpiryDate()   const { return expiryDate; }
    int         getQuantity()     const { return quantity; }
    double      getPrice()        const { return pricePerUnit; }
    int         getReorderLevel() const { return reorderLevel; }

    void setQuantity(int q)    { quantity = q; }
    void setPrice(double p)    { pricePerUnit = p; }
    void setReorder(int r)     { reorderLevel = r; }

    // Check if expired relative to today
    bool isExpired(const Date& today) const {
        return expiryDate < today;
    }

    // Days until expiry (negative = already expired)
    int daysToExpiry(const Date& today) const {
        return expiryDate.daysFrom(today);
    }

    bool isLowStock() const {
        return quantity <= reorderLevel;
    }

    double totalValue() const {
        return quantity * pricePerUnit;
    }

    void display(const Date& today) const {
        int dte = daysToExpiry(today);
        string status;
        if (dte < 0)        status = "*** EXPIRED ***";
        else if (dte <= 30) status = "!! EXPIRING SOON !!";
        else if (dte <= 90) status = "Near Expiry";
        else                status = "OK";

        cout << left
             << setw(5)  << id
             << setw(22) << name
             << setw(14) << category
             << setw(10) << quantity
             << setw(14) << expiryDate.toString()
             << setw(10) << fixed << setprecision(2) << pricePerUnit
             << setw(20) << status;
        if (isLowStock()) cout << " [LOW STOCK]";
        cout << "\n";
    }
};

int Medicine::idCounter = 0;

// ─────────────────────────────────────────────
//  Transaction Log
// ─────────────────────────────────────────────
struct Transaction {
    int    medicineId;
    string medicineName;
    string type;   // "PURCHASE", "SALE", "DISPOSAL"
    int    units;
    double amount;
    Date   date;
    string note;
};

// ─────────────────────────────────────────────
//  Pharmacy Class
// ─────────────────────────────────────────────
class Pharmacy {
private:
    string pharmacyName;
    vector<Medicine> inventory;
    vector<Transaction> transactions;
    Date today;

    Medicine* findById(int id) {
        for (auto& m : inventory)
            if (m.getId() == id) return &m;
        return nullptr;
    }

    void printHeader(const string& title) const {
        cout << "\n" << string(90, '=') << "\n";
        cout << "  " << title << "\n";
        cout << string(90, '=') << "\n";
    }

    void printTableHeader() const {
        cout << left
             << setw(5)  << "ID"
             << setw(22) << "Medicine Name"
             << setw(14) << "Category"
             << setw(10) << "Stock"
             << setw(14) << "Expiry"
             << setw(10) << "Price(₹)"
             << setw(20) << "Status"
             << "\n"
             << string(90, '-') << "\n";
    }

public:
    Pharmacy(const string& name, Date currentDate)
        : pharmacyName(name), today(currentDate) {}

    // ── Add Medicine ──────────────────────────
    void addMedicine(Medicine m) {
        inventory.push_back(m);
        cout << "[+] Medicine '" << m.getName()
             << "' added with ID " << m.getId() << ".\n";
    }

    // ── Restock (Purchase) ────────────────────
    bool restock(int id, int units, double costPerUnit) {
        Medicine* m = findById(id);
        if (!m) { cout << "[-] Medicine ID " << id << " not found.\n"; return false; }
        m->setQuantity(m->getQuantity() + units);
        Transaction t{id, m->getName(), "PURCHASE", units,
                      units * costPerUnit, today, "Restock"};
        transactions.push_back(t);
        cout << "[+] Restocked " << units << " units of '" << m->getName() << "'.\n";
        return true;
    }

    // ── Sell ─────────────────────────────────
    bool sell(int id, int units) {
        Medicine* m = findById(id);
        if (!m) { cout << "[-] Medicine ID " << id << " not found.\n"; return false; }
        if (m->isExpired(today)) {
            cout << "[!] Cannot sell '" << m->getName() << "' — it is EXPIRED.\n";
            return false;
        }
        if (m->getQuantity() < units) {
            cout << "[!] Insufficient stock. Available: " << m->getQuantity() << "\n";
            return false;
        }
        m->setQuantity(m->getQuantity() - units);
        double revenue = units * m->getPrice();
        Transaction t{id, m->getName(), "SALE", units, revenue, today, "Sale"};
        transactions.push_back(t);
        cout << "[+] Sold " << units << " units of '" << m->getName()
             << "'. Revenue: ₹" << fixed << setprecision(2) << revenue << "\n";
        if (m->isLowStock())
            cout << "[!] WARNING: Stock for '" << m->getName()
                 << "' is below reorder level (" << m->getReorderLevel() << ")!\n";
        return true;
    }

    // ── Dispose Expired ───────────────────────
    void disposeExpired() {
        printHeader("DISPOSE EXPIRED MEDICINES");
        int count = 0;
        for (auto& m : inventory) {
            if (m.isExpired(today) && m.getQuantity() > 0) {
                cout << "Disposing " << m.getQuantity()
                     << " units of '" << m.getName()
                     << "' (Expired: " << m.getExpiryDate().toString() << ")\n";
                Transaction t{m.getId(), m.getName(), "DISPOSAL",
                              m.getQuantity(), 0.0, today, "Expired"};
                transactions.push_back(t);
                m.setQuantity(0);
                count++;
            }
        }
        if (count == 0) cout << "No expired medicines to dispose.\n";
        else cout << "[+] Disposed " << count << " expired medicine batch(es).\n";
    }

    // ── View All Inventory ────────────────────
    void viewInventory() const {
        printHeader("FULL INVENTORY — " + pharmacyName);
        printTableHeader();
        if (inventory.empty()) { cout << "  (No medicines in inventory)\n"; return; }
        for (const auto& m : inventory)
            m.display(today);
    }

    // ── Expiry Alerts ─────────────────────────
    void expiryAlerts(int withinDays = 90) const {
        printHeader("EXPIRY ALERTS (within " + to_string(withinDays) + " days)");
        printTableHeader();
        bool found = false;
        // Sort a copy by expiry
        vector<const Medicine*> ptrs;
        for (const auto& m : inventory)
            if (!m.isExpired(today) && m.daysToExpiry(today) <= withinDays)
                ptrs.push_back(&m);
        sort(ptrs.begin(), ptrs.end(), [](const Medicine* a, const Medicine* b){
            return a->getExpiryDate() < b->getExpiryDate();
        });
        for (auto* m : ptrs) { m->display(today); found = true; }
        if (!found) cout << "  No medicines expiring within " << withinDays << " days.\n";
    }

    // ── Expired List ──────────────────────────
    void viewExpired() const {
        printHeader("EXPIRED MEDICINES");
        printTableHeader();
        bool found = false;
        for (const auto& m : inventory)
            if (m.isExpired(today)) { m.display(today); found = true; }
        if (!found) cout << "  No expired medicines in inventory.\n";
    }

    // ── Low Stock Report ──────────────────────
    void lowStockReport() const {
        printHeader("LOW STOCK REPORT");
        printTableHeader();
        bool found = false;
        for (const auto& m : inventory)
            if (m.isLowStock()) { m.display(today); found = true; }
        if (!found) cout << "  All medicines are adequately stocked.\n";
    }

    // ── Search by Name ────────────────────────
    void searchByName(const string& keyword) const {
        printHeader("SEARCH RESULTS: \"" + keyword + "\"");
        printTableHeader();
        bool found = false;
        for (const auto& m : inventory) {
            string name = m.getName();
            // Case-insensitive match
            string lName = name, lKey = keyword;
            transform(lName.begin(), lName.end(), lName.begin(), ::tolower);
            transform(lKey.begin(), lKey.end(), lKey.begin(), ::tolower);
            if (lName.find(lKey) != string::npos) {
                m.display(today); found = true;
            }
        }
        if (!found) cout << "  No medicine found matching '" << keyword << "'.\n";
    }

    // ── Transaction Log ───────────────────────
    void viewTransactions() const {
        printHeader("TRANSACTION LOG");
        cout << left
             << setw(6)  << "Med ID"
             << setw(22) << "Medicine"
             << setw(12) << "Type"
             << setw(8)  << "Units"
             << setw(12) << "Amount(₹)"
             << setw(14) << "Date"
             << "Note\n"
             << string(90, '-') << "\n";
        for (const auto& t : transactions) {
            cout << left
                 << setw(6)  << t.medicineId
                 << setw(22) << t.medicineName
                 << setw(12) << t.type
                 << setw(8)  << t.units
                 << setw(12) << fixed << setprecision(2) << t.amount
                 << setw(14) << t.date.toString()
                 << t.note << "\n";
        }
        if (transactions.empty()) cout << "  No transactions recorded.\n";
    }

    // ── Summary Dashboard ─────────────────────
    void dashboard() const {
        printHeader("PHARMACY DASHBOARD — " + pharmacyName);
        int total = inventory.size();
        int expired = 0, expiringSoon = 0, lowStock = 0;
        double totalVal = 0;
        for (const auto& m : inventory) {
            if (m.isExpired(today))                       expired++;
            else if (m.daysToExpiry(today) <= 30)         expiringSoon++;
            if (m.isLowStock())                           lowStock++;
            totalVal += m.totalValue();
        }
        cout << "  Date              : " << today.toString()         << "\n";
        cout << "  Total Medicines   : " << total                    << "\n";
        cout << "  Expired           : " << expired                  << "\n";
        cout << "  Expiring ≤30 days : " << expiringSoon             << "\n";
        cout << "  Low Stock Items   : " << lowStock                 << "\n";
        cout << "  Inventory Value   : ₹" << fixed << setprecision(2)
             << totalVal                                             << "\n";
        cout << "  Transactions      : " << transactions.size()      << "\n";
        cout << string(90, '-') << "\n";
    }
};

// ─────────────────────────────────────────────
//  Helper: Input Validation
// ─────────────────────────────────────────────
int getInt(const string& prompt) {
    int val;
    while (true) {
        cout << prompt;
        if (cin >> val) { cin.ignore(); return val; }
        cout << "[!] Invalid input. Enter a number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

double getDouble(const string& prompt) {
    double val;
    while (true) {
        cout << prompt;
        if (cin >> val) { cin.ignore(); return val; }
        cout << "[!] Invalid input.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

Date getDate(const string& prompt) {
    int d, m, y;
    cout << prompt << " (DD MM YYYY): ";
    cin >> d >> m >> y;
    cin.ignore();
    return Date(d, m, y);
}

string getString(const string& prompt) {
    string s;
    cout << prompt;
    getline(cin, s);
    return s;
}

// ─────────────────────────────────────────────
//  Main Menu
// ─────────────────────────────────────────────
void showMenu() {
    cout << "\n╔══════════════════════════════════════╗\n";
    cout << "║   PHARMACY MANAGEMENT SYSTEM MENU   ║\n";
    cout << "╠══════════════════════════════════════╣\n";
    cout << "║  1. Dashboard                        ║\n";
    cout << "║  2. View Full Inventory              ║\n";
    cout << "║  3. Add New Medicine                 ║\n";
    cout << "║  4. Restock Medicine                 ║\n";
    cout << "║  5. Sell Medicine                    ║\n";
    cout << "║  6. Expiry Alerts (90 days)          ║\n";
    cout << "║  7. View Expired Medicines           ║\n";
    cout << "║  8. Dispose Expired Medicines        ║\n";
    cout << "║  9. Low Stock Report                 ║\n";
    cout << "║ 10. Search Medicine by Name          ║\n";
    cout << "║ 11. Transaction Log                  ║\n";
    cout << "║  0. Exit                             ║\n";
    cout << "╚══════════════════════════════════════╝\n";
    cout << "Enter choice: ";
}

// ─────────────────────────────────────────────
//  Sample Data Loader
// ─────────────────────────────────────────────
void loadSampleData(Pharmacy& p) {
    // Format: name, category, manufacturer, mfgDate, expiryDate, qty, price, reorderLevel
    p.addMedicine(Medicine("Paracetamol 500mg", "Analgesic",  "Sun Pharma",
        Date(1,1,2023), Date(1,6,2025), 200, 2.50, 50));
    p.addMedicine(Medicine("Amoxicillin 250mg", "Antibiotic", "Cipla",
        Date(1,3,2023), Date(1,3,2026), 150, 8.00, 30));
    p.addMedicine(Medicine("Cetirizine 10mg",   "Antihistamine","Abbott",
        Date(1,6,2023), Date(1,9,2025), 80,  5.50, 20));
    p.addMedicine(Medicine("Vitamin C 500mg",   "Supplement",  "Himalaya",
        Date(1,2,2024), Date(1,2,2027), 300, 3.00, 60));
    p.addMedicine(Medicine("Metformin 500mg",   "Antidiabetic","Dr. Reddy's",
        Date(1,4,2023), Date(31,3,2025), 10,  6.00, 25)); // Low stock + near expiry
    p.addMedicine(Medicine("Omeprazole 20mg",   "Antacid",    "Zydus",
        Date(1,1,2022), Date(1,1,2025), 50,  4.50, 15)); // Already expired
    p.addMedicine(Medicine("Atorvastatin 10mg", "Cardiac",    "Lupin",
        Date(1,5,2024), Date(1,5,2027), 120, 12.00, 25));
    p.addMedicine(Medicine("Azithromycin 500mg","Antibiotic", "Macleods",
        Date(1,7,2023), Date(1,7,2026), 60,  15.00, 15));
}

// ─────────────────────────────────────────────
//  Main
// ─────────────────────────────────────────────
int main() {
    cout << "\n╔══════════════════════════════════════════════╗\n";
    cout << "║   PHARMACY INVENTORY & EXPIRY MANAGEMENT    ║\n";
    cout << "║            Case Study | C++                 ║\n";
    cout << "╚══════════════════════════════════════════════╝\n";

    // Today's date for demo: 15 March 2025
    Date today(15, 3, 2025);
    Pharmacy pharmacy("HealthPlus Pharmacy", today);

    // Load sample medicines
    loadSampleData(pharmacy);

    int choice;
    do {
        showMenu();
        cin >> choice;
        cin.ignore();

        switch (choice) {
        case 1:
            pharmacy.dashboard();
            break;
        case 2:
            pharmacy.viewInventory();
            break;
        case 3: {
            string name     = getString("Medicine Name    : ");
            string category = getString("Category         : ");
            string mfr      = getString("Manufacturer     : ");
            Date mfgDate    = getDate("Manufacture Date ");
            Date expDate    = getDate("Expiry Date      ");
            int qty         = getInt("Quantity         : ");
            double price    = getDouble("Price per unit   : ");
            int reorder     = getInt("Reorder Level    : ");
            pharmacy.addMedicine(Medicine(name, category, mfr, mfgDate, expDate, qty, price, reorder));
            break;
        }
        case 4: {
            int id    = getInt("Medicine ID to restock : ");
            int units = getInt("Units to add           : ");
            double cost = getDouble("Cost per unit        : ");
            pharmacy.restock(id, units, cost);
            break;
        }
        case 5: {
            int id    = getInt("Medicine ID to sell : ");
            int units = getInt("Units to sell       : ");
            pharmacy.sell(id, units);
            break;
        }
        case 6:
            pharmacy.expiryAlerts(90);
            break;
        case 7:
            pharmacy.viewExpired();
            break;
        case 8:
            pharmacy.disposeExpired();
            break;
        case 9:
            pharmacy.lowStockReport();
            break;
        case 10: {
            string kw = getString("Enter medicine name / keyword: ");
            pharmacy.searchByName(kw);
            break;
        }
        case 11:
            pharmacy.viewTransactions();
            break;
        case 0:
            cout << "\nExiting. Stay Healthy!\n";
            break;
        default:
            cout << "[!] Invalid choice. Try again.\n";
        }
    } while (choice != 0);

    return 0;
}

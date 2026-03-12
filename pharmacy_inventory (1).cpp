#include <iostream>
#include <fstream>
using namespace std;

struct Medicine {
    int id;
    string name;
    int quantity;
    float price;

    int expDay;
    int expMonth;
    int expYear;
};

Medicine medicines[50];
int countMed = 0;

int todayDay, todayMonth, todayYear;

bool isExpired(Medicine m) {

    if(m.expYear < todayYear)
        return true;

    if(m.expYear == todayYear && m.expMonth < todayMonth)
        return true;

    if(m.expYear == todayYear && m.expMonth == todayMonth && m.expDay < todayDay)
        return true;

    return false;
}

void saveToFile() {

    ofstream file("medicines.txt");

    for(int i=0;i<countMed;i++) {

        file << medicines[i].id << " "
             << medicines[i].name << " "
             << medicines[i].quantity << " "
             << medicines[i].price << " "
             << medicines[i].expDay << " "
             << medicines[i].expMonth << " "
             << medicines[i].expYear << endl;
    }

    file.close();
}

void loadFromFile() {

    ifstream file("medicines.txt");

    while(file >> medicines[countMed].id
               >> medicines[countMed].name
               >> medicines[countMed].quantity
               >> medicines[countMed].price
               >> medicines[countMed].expDay
               >> medicines[countMed].expMonth
               >> medicines[countMed].expYear) {

        countMed++;
    }

    file.close();
}

void addMedicine() {

    Medicine m;

    cout << "Enter Medicine ID: ";
    cin >> m.id;

    cout << "Enter Medicine Name: ";
    cin >> m.name;

    cout << "Enter Quantity: ";
    cin >> m.quantity;

    cout << "Enter Price: ";
    cin >> m.price;

    cout << "Enter Expiry Day: ";
    cin >> m.expDay;

    cout << "Enter Expiry Month: ";
    cin >> m.expMonth;

    cout << "Enter Expiry Year: ";
    cin >> m.expYear;

    medicines[countMed] = m;
    countMed++;

    saveToFile();

    cout << "Medicine Added Successfully\n";
}

void viewMedicines() {

    if(countMed == 0) {
        cout << "No medicines available\n";
        return;
    }

    for(int i=0;i<countMed;i++) {

        cout << "\nID: " << medicines[i].id;
        cout << "\nName: " << medicines[i].name;
        cout << "\nQuantity: " << medicines[i].quantity;
        cout << "\nPrice: " << medicines[i].price;

        cout << "\nExpiry: "
             << medicines[i].expDay << "/"
             << medicines[i].expMonth << "/"
             << medicines[i].expYear;

        if(isExpired(medicines[i]))
            cout << "  --> EXPIRED";

        cout << "\n-----------------------\n";
    }
}

void sellMedicine() {

    int id, qty;

    cout << "Enter Medicine ID: ";
    cin >> id;

    cout << "Enter Quantity to sell: ";
    cin >> qty;

    for(int i=0;i<countMed;i++) {

        if(medicines[i].id == id) {

            if(isExpired(medicines[i])) {
                cout << "Cannot sell. Medicine is EXPIRED\n";
                return;
            }

            if(medicines[i].quantity >= qty) {

                medicines[i].quantity -= qty;
                saveToFile();

                cout << "Medicine sold successfully\n";

            } else {
                cout << "Not enough stock\n";
            }

            return;
        }
    }

    cout << "Medicine not found\n";
}

int main() {

    loadFromFile();

    cout << "Enter Today's Date\n";
    cout << "Day: ";
    cin >> todayDay;

    cout << "Month: ";
    cin >> todayMonth;

    cout << "Year: ";
    cin >> todayYear;

    int choice;

    do {

        cout << "\nPHARMACY SYSTEM\n";
        cout << "1 Add Medicine\n";
        cout << "2 View Medicines\n";
        cout << "3 Sell Medicine\n";
        cout << "0 Exit\n";

        cout << "Enter choice: ";
        cin >> choice;

        switch(choice) {

            case 1:
                addMedicine();
                break;

            case 2:
                viewMedicines();
                break;

            case 3:
                sellMedicine();
                break;

            case 0:
                cout << "Exiting program\n";
                break;

            default:
                cout << "Invalid choice\n";
        }

    } while(choice != 0);

    return 0;
}

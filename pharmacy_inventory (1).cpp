#include <iostream>
#include <fstream>
using namespace std;

struct Medicine {
    int id;
    string name;
    int quantity;
    float price;

    int expDay, expMonth, expYear;
};

Medicine med[50];
int countMed = 0;

int todayDay, todayMonth, todayYear;

// Check if medicine expired
bool isExpired(Medicine m)
{
    if(m.expYear < todayYear)
        return true;

    if(m.expYear == todayYear && m.expMonth < todayMonth)
        return true;

    if(m.expYear == todayYear && m.expMonth == todayMonth && m.expDay < todayDay)
        return true;

    return false;
}

// Save medicines to file
void saveFile()
{
    ofstream file("medicines.txt");

    for(int i=0;i<countMed;i++)
    {
        file << med[i].id << " "
             << med[i].name << " "
             << med[i].quantity << " "
             << med[i].price << " "
             << med[i].expDay << " "
             << med[i].expMonth << " "
             << med[i].expYear << endl;
    }

    file.close();
}

// Load medicines from file
void loadFile()
{
    ifstream file("medicines.txt");

    while(file >> med[countMed].id
               >> med[countMed].name
               >> med[countMed].quantity
               >> med[countMed].price
               >> med[countMed].expDay
               >> med[countMed].expMonth
               >> med[countMed].expYear)
    {
        countMed++;
    }

    file.close();
}

// Add medicine
void addMedicine()
{
    cout << "\nEnter Medicine ID: ";
    cin >> med[countMed].id;

    cout << "Enter Medicine Name: ";
    cin >> med[countMed].name;

    cout << "Enter Quantity: ";
    cin >> med[countMed].quantity;

    cout << "Enter Price: ";
    cin >> med[countMed].price;

    cout << "Enter Expiry Day: ";
    cin >> med[countMed].expDay;

    cout << "Enter Expiry Month: ";
    cin >> med[countMed].expMonth;

    cout << "Enter Expiry Year: ";
    cin >> med[countMed].expYear;

    countMed++;

    saveFile();

    cout << "Medicine Added Successfully\n";
}

// View medicines
void viewMedicines()
{
    if(countMed == 0)
    {
        cout << "No medicines available\n";
        return;
    }

    for(int i=0;i<countMed;i++)
    {
        cout << "\nID: " << med[i].id;
        cout << "\nName: " << med[i].name;
        cout << "\nQuantity: " << med[i].quantity;
        cout << "\nPrice: " << med[i].price;

        cout << "\nExpiry: "
             << med[i].expDay << "/"
             << med[i].expMonth << "/"
             << med[i].expYear;

        if(isExpired(med[i]))
            cout << "  -> EXPIRED";

        cout << "\n------------------\n";
    }
}

// Sell medicine
void sellMedicine()
{
    int id, qty;

    cout << "Enter Medicine ID: ";
    cin >> id;

    cout << "Enter Quantity to sell: ";
    cin >> qty;

    for(int i=0;i<countMed;i++)
    {
        if(med[i].id == id)
        {
            if(isExpired(med[i]))
            {
                cout << "Cannot sell. Medicine expired\n";
                return;
            }

            if(med[i].quantity >= qty)
            {
                med[i].quantity -= qty;

                saveFile();

                cout << "Medicine Sold Successfully\n";
            }
            else
            {
                cout << "Not enough stock\n";
            }

            return;
        }
    }

    cout << "Medicine not found\n";
}

int main()
{
    loadFile();

    cout << "Enter Today's Date\n";
    cout << "Day: ";
    cin >> todayDay;

    cout << "Month: ";
    cin >> todayMonth;

    cout << "Year: ";
    cin >> todayYear;

    int choice;

    do
    {
        cout << "\n--- PHARMACY SYSTEM ---\n";
        cout << "1. Add Medicine\n";
        cout << "2. View Medicines\n";
        cout << "3. Sell Medicine\n";
        cout << "0. Exit\n";

        cout << "Enter Choice: ";
        cin >> choice;

        switch(choice)
        {
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
                cout << "Exiting...\n";
                break;

            default:
                cout << "Invalid choice\n";
        }

    } while(choice != 0);

    return 0;
}

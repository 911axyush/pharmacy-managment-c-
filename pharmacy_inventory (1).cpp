#include <iostream>
using namespace std;
 
struct Date {
    int day, month, year;
    Date(int d=1,int m=1,int y=2024):day(d),month(m),year(y){}
 
    void print() const {
        if(day<10) cout<<'0'; cout<<day<<'/';
        if(month<10) cout<<'0'; cout<<month<<'/'<<year;
    }
 
    bool operator<(const Date& o) const {
        if(year!=o.year) return year<o.year;
        if(month!=o.month) return month<o.month;
        return day<o.day;
    }
 
    int daysFrom(const Date& o) const {
        auto jd=[](int d,int m,int y)->long{
            int a=(14-m)/12, yy=y+4800-a, mm=m+12*a-3;
            return d+(153*mm+2)/5+365L*yy+yy/4-yy/100+yy/400-32045;
        };
        return (int)(jd(day,month,year)-jd(o.day,o.month,o.year));
    }
};
 

const int SLEN = 64;
void scopy(char* dst, const char* src) {
    int i=0; while(src[i]&&i<SLEN-1){dst[i]=src[i];i++;} dst[i]='\0';
}
bool scontains(const char* haystack, const char* needle) {
    // case-insensitive substring search
    for(int i=0; haystack[i]; i++) {
        int j=0;
        while(needle[j] && haystack[i+j] &&
             (haystack[i+j]|32)==(needle[j]|32)) j++;
        if(!needle[j]) return true;
    }
    return false;
}
void pad(const char* s, int w) {
    int len=0; while(s[len]) len++;
    cout<<s;
    for(int i=len;i<w;i++) cout<<' ';
}
void padInt(int v, int w) {
    // print integer left-padded to width w
    char buf[16]; int i=0;
    if(v<0){buf[i++]='-'; v=-v;}
    if(v==0){buf[i++]='0';}
    else{ int tmp=v,digits=0; while(tmp){digits++;tmp/=10;}
          for(int k=digits-1;k>=0;k--){buf[i+k]='0'+v%10;v/=10;} i+=digits; }
    buf[i]='\0'; pad(buf,w);
}
void padDbl(double v, int w) {
    int whole=(int)v; int frac=(int)((v-(int)v)*100+0.5);
    char buf[24]; int i=0;
    int tmp=whole,digits=0; if(whole==0)digits=1;
    else while(tmp){digits++;tmp/=10;}
    for(int k=digits-1;k>=0;k--){buf[i+k]='0'+whole%10;whole/=10;} i+=digits;
    buf[i++]='.';
    buf[i++]='0'+frac/10; buf[i++]='0'+frac%10; buf[i]='\0';
    pad(buf,w);
}
 
// ── Medicine ──────────────────────────────────────────────────────────
const int MAX = 50;
struct Medicine {
    static int cnt;
    int id,qty,reorder; double price;
    char name[SLEN],cat[SLEN],mfr[SLEN];
    Date mfgD,expD;
 
    Medicine(){}
    Medicine(const char* n,const char* c,const char* m,Date md,Date ed,int q,double p,int r)
        :id(++cnt),qty(q),reorder(r),price(p),mfgD(md),expD(ed){ scopy(name,n);scopy(cat,c);scopy(mfr,m); }
 
    bool isExpired(const Date& t)  const { return expD<t; }
    int  daysToExp(const Date& t)  const { return expD.daysFrom(t); }
    bool isLowStock()              const { return qty<=reorder; }
    double totalVal()              const { return qty*price; }
 
    void display(const Date& t) const {
        int d=daysToExp(t);
        const char* s = d<0?"*** EXPIRED ***":d<=30?"!! EXPIRING SOON":d<=90?"Near Expiry":"OK";
        padInt(id,5); pad(name,22); pad(cat,14);
        padInt(qty,8); expD.print(); cout<<"  "; padDbl(price,10); pad(s,18);
        if(isLowStock()) cout<<" [LOW STOCK]";
        cout<<"\n";
    }
};
int Medicine::cnt=0;
 
// ── Transaction ───────────────────────────────────────────────────────
struct Transaction {
    int medId,units; double amt; Date date;
    char medName[SLEN],type[16],note[32];
};
 
// ── Pharmacy ──────────────────────────────────────────────────────────
struct Pharmacy {
    char pname[SLEN];
    Medicine inv[MAX]; int invSz=0;
    Transaction txn[MAX*4]; int txnSz=0;
    Date today;
 
    Pharmacy(const char* n, Date d){ scopy(pname,n); today=d; }
 
    Medicine* find(int id){ for(int i=0;i<invSz;i++) if(inv[i].id==id) return &inv[i]; return nullptr; }
 
    void hdr(const char* t) const {
        cout<<"\n"; for(int i=0;i<80;i++) cout<<'='; cout<<"\n  "<<t<<"\n"; for(int i=0;i<80;i++) cout<<'='; cout<<"\n";
    }
    void colHdr() const {
        pad("ID",5);pad("Name",22);pad("Category",14);pad("Stock",8);
        pad("Expiry",16);pad("Price",10);cout<<"Status\n";
        for(int i=0;i<80;i++) cout<<'-'; cout<<"\n";
    }
 
    void addMed(Medicine m){
        if(invSz>=MAX){cout<<"[!] Inventory full\n";return;}
        inv[invSz++]=m;
        cout<<"[+] '"<<m.name<<"' added (ID "<<m.id<<")\n";
    }
 
    bool restock(int id,int u,double cost){
        Medicine* m=find(id);
        if(!m){cout<<"[-] ID "<<id<<" not found\n";return false;}
        m->qty+=u;
        Transaction t; t.medId=id; scopy(t.medName,m->name); scopy(t.type,"PURCHASE");
        t.units=u; t.amt=u*cost; t.date=today; scopy(t.note,"Restock");
        txn[txnSz++]=t;
        cout<<"[+] Restocked "<<u<<" units of '"<<m->name<<"'\n"; return true;
    }
 
    bool sell(int id,int u){
        Medicine* m=find(id);
        if(!m){cout<<"[-] ID "<<id<<" not found\n";return false;}
        if(m->isExpired(today)){cout<<"[!] Cannot sell '"<<m->name<<"' — EXPIRED\n";return false;}
        if(m->qty<u){cout<<"[!] Insufficient stock. Available: "<<m->qty<<"\n";return false;}
        m->qty-=u; double rev=u*m->price;
        Transaction t; t.medId=id; scopy(t.medName,m->name); scopy(t.type,"SALE");
        t.units=u; t.amt=rev; t.date=today; scopy(t.note,"Sale");
        txn[txnSz++]=t;
        cout<<"[+] Sold "<<u<<" units of '"<<m->name<<"'. Revenue: Rs."; padDbl(rev,1); cout<<"\n";
        if(m->isLowStock()) cout<<"[!] Low stock warning: '"<<m->name<<"' (reorder="<<m->reorder<<")\n";
        return true;
    }
 
    void disposeExpired(){
        hdr("DISPOSE EXPIRED MEDICINES"); int c=0;
        for(int i=0;i<invSz;i++) if(inv[i].isExpired(today)&&inv[i].qty>0){
            cout<<"Disposing "<<inv[i].qty<<" units of '"<<inv[i].name<<"'\n";
            Transaction t; t.medId=inv[i].id; scopy(t.medName,inv[i].name); scopy(t.type,"DISPOSAL");
            t.units=inv[i].qty; t.amt=0; t.date=today; scopy(t.note,"Expired");
            txn[txnSz++]=t; inv[i].qty=0; c++;
        }
        if(c==0) cout<<"  No expired stock to dispose\n";
        else cout<<"[+] Disposed "<<c<<" batch(es)\n";
    }
 
    void viewInventory() const {
        hdr("FULL INVENTORY"); colHdr();
        if(invSz==0){cout<<"  (empty)\n";return;}
        for(int i=0;i<invSz;i++) inv[i].display(today);
    }
 
    void expiryAlerts(int days) const {
        hdr("EXPIRY ALERTS"); colHdr();
        // collect pointers, bubble sort by expiry
        const Medicine* p[MAX]; int n=0;
        for(int i=0;i<invSz;i++)
            if(!inv[i].isExpired(today)&&inv[i].daysToExp(today)<=days) p[n++]=&inv[i];
        for(int i=0;i<n-1;i++) for(int j=0;j<n-1-i;j++)
            if(p[j+1]->expD<p[j]->expD){ const Medicine* tmp=p[j];p[j]=p[j+1];p[j+1]=tmp; }
        if(n==0) cout<<"  No medicines expiring within "<<days<<" days\n";
        else for(int i=0;i<n;i++) p[i]->display(today);
    }
 
    void viewExpired() const {
        hdr("EXPIRED MEDICINES"); colHdr(); bool f=false;
        for(int i=0;i<invSz;i++) if(inv[i].isExpired(today)){inv[i].display(today);f=true;}
        if(!f) cout<<"  No expired medicines\n";
    }
 
    void lowStockReport() const {
        hdr("LOW STOCK REPORT"); colHdr(); bool f=false;
        for(int i=0;i<invSz;i++) if(inv[i].isLowStock()){inv[i].display(today);f=true;}
        if(!f) cout<<"  All medicines adequately stocked\n";
    }
 
    void searchByName(const char* kw) const {
        hdr("SEARCH RESULTS"); colHdr(); bool f=false;
        for(int i=0;i<invSz;i++) if(scontains(inv[i].name,kw)){inv[i].display(today);f=true;}
        if(!f) cout<<"  No match found for '"<<kw<<"'\n";
    }
 
    void viewTxn() const {
        hdr("TRANSACTION LOG");
        pad("ID",6);pad("Medicine",22);pad("Type",12);pad("Units",7);pad("Amount",12);pad("Date",14);cout<<"Note\n";
        for(int i=0;i<80;i++) cout<<'-'; cout<<"\n";
        if(txnSz==0){cout<<"  No transactions recorded\n";return;}
        for(int i=0;i<txnSz;i++){
            padInt(txn[i].medId,6); pad(txn[i].medName,22); pad(txn[i].type,12);
            padInt(txn[i].units,7); padDbl(txn[i].amt,12);
            txn[i].date.print(); cout<<"  "<<txn[i].note<<"\n";
        }
    }
 
    void dashboard() const {
        hdr("PHARMACY DASHBOARD"); int exp=0,soon=0,low=0; double val=0;
        for(int i=0;i<invSz;i++){
            if(inv[i].isExpired(today)) exp++;
            else if(inv[i].daysToExp(today)<=30) soon++;
            if(inv[i].isLowStock()) low++; val+=inv[i].totalVal();
        }
        cout<<"  Date            : "; today.print(); cout<<"\n";
        cout<<"  Total Medicines : "<<invSz<<"\n";
        cout<<"  Expired         : "<<exp<<"\n";
        cout<<"  Expiring <=30d  : "<<soon<<"\n";
        cout<<"  Low Stock       : "<<low<<"\n";
        cout<<"  Inventory Value : Rs."; padDbl(val,1); cout<<"\n";
        cout<<"  Transactions    : "<<txnSz<<"\n";
        for(int i=0;i<80;i++) cout<<'-'; cout<<"\n";
    }
};
 
// ── Input helpers ─────────────────────────────────────────────────────
int getInt(const char* p){
    int v; while(true){cout<<p; if(cin>>v){cin.ignore(1000,'\n');return v;}
    cout<<"[!] Enter a number\n"; cin.clear(); cin.ignore(1000,'\n');}
}
double getDbl(const char* p){
    double v; while(true){cout<<p; if(cin>>v){cin.ignore(1000,'\n');return v;}
    cout<<"[!] Enter a number\n"; cin.clear(); cin.ignore(1000,'\n');}
}
Date getDate(const char* p){ int d,m,y; cout<<p<<" (DD MM YYYY): "; cin>>d>>m>>y; cin.ignore(1000,'\n'); return Date(d,m,y); }
void getStr(const char* p, char* buf){
    cout<<p;
    int i=0; char c;
    while(cin.get(c)&&c!='\n'&&i<SLEN-1) buf[i++]=c;
    buf[i]='\0';
}
 
// ── Sample data ───────────────────────────────────────────────────────
void loadData(Pharmacy& p){
    p.addMed(Medicine("Paracetamol 500mg", "Analgesic",    "Sun Pharma",  Date(1,1,2023),Date(1,6,2025), 200, 2.50,50));
    p.addMed(Medicine("Amoxicillin 250mg", "Antibiotic",   "Cipla",       Date(1,3,2023),Date(1,3,2026), 150, 8.00,30));
    p.addMed(Medicine("Cetirizine 10mg",   "Antihistamine","Abbott",      Date(1,6,2023),Date(1,9,2025),  80, 5.50,20));
    p.addMed(Medicine("Vitamin C 500mg",   "Supplement",   "Himalaya",    Date(1,2,2024),Date(1,2,2027), 300, 3.00,60));
    p.addMed(Medicine("Metformin 500mg",   "Antidiabetic", "Dr. Reddy's", Date(1,4,2023),Date(31,3,2025), 10, 6.00,25));
    p.addMed(Medicine("Omeprazole 20mg",   "Antacid",      "Zydus",       Date(1,1,2022),Date(1,1,2025),  50, 4.50,15));
    p.addMed(Medicine("Atorvastatin 10mg", "Cardiac",      "Lupin",       Date(1,5,2024),Date(1,5,2027), 120,12.00,25));
    p.addMed(Medicine("Azithromycin 500mg","Antibiotic",   "Macleods",    Date(1,7,2023),Date(1,7,2026),  60,15.00,15));
}
 
// ── Main ──────────────────────────────────────────────────────────────
int main(){
    cout<<"╔══════════════════════════════════════════════╗\n"
        <<"║   PHARMACY INVENTORY & EXPIRY MANAGEMENT     ║\n"
        <<"║  Aayush Jadhav  |  Roll No: 150096725008     ║\n"
        <<"╚══════════════════════════════════════════════╝\n";
 
    Pharmacy ph("HealthPlus Pharmacy", Date(15,3,2025));
    loadData(ph);
 
    const char* menu[]={"Dashboard","View Full Inventory","Add Medicine","Restock",
                        "Sell","Expiry Alerts (90d)","View Expired","Dispose Expired",
                        "Low Stock Report","Search by Name","Transaction Log"};
    int ch;
    do {
        cout<<"\n╔══════════════════════════════╗\n║  PHARMACY MANAGEMENT MENU    ║\n╠══════════════════════════════╣\n";
        for(int i=0;i<11;i++){
            cout<<"║  "; if(i+1<10) cout<<' '; cout<<(i+1)<<". "; pad(menu[i],24); cout<<"║\n";
        }
        cout<<"║   0. "; pad("Exit",24); cout<<"║\n╚══════════════════════════════╝\nChoice: ";
        cin>>ch; cin.ignore(1000,'\n');
 
        char buf[SLEN];
        switch(ch){
        case  1: ph.dashboard();          break;
        case  2: ph.viewInventory();      break;
        case  3: {
            char n[SLEN],c[SLEN],m[SLEN];
            getStr("Name         : ",n); getStr("Category     : ",c); getStr("Manufacturer : ",m);
            Date md=getDate("Mfg Date "), ed=getDate("Expiry Date ");
            ph.addMed(Medicine(n,c,m,md,ed,getInt("Qty     : "),getDbl("Price   : "),getInt("Reorder : ")));
            break; }
        case  4: ph.restock(getInt("ID: "),getInt("Units: "),getDbl("Cost/unit: ")); break;
        case  5: ph.sell(getInt("ID: "),getInt("Units: "));   break;
        case  6: ph.expiryAlerts(90);     break;
        case  7: ph.viewExpired();        break;
        case  8: ph.disposeExpired();     break;
        case  9: ph.lowStockReport();     break;
        case 10: getStr("Keyword: ",buf); ph.searchByName(buf); break;
        case 11: ph.viewTxn();            break;
        case  0: cout<<"Exiting. Stay Healthy!\n"; break;
        default: cout<<"[!] Invalid choice\n";
        }
    } while(ch!=0);
}
 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ===== Structures ===== */
struct User {
    int accountNo;
    char name[30];
    int pin;               // 4-digit PIN
    double balance;        // safer for money
    long atmCardNo;
    int cardStatus;        // 1 = ACTIVE, 0 = BLOCKED
    char cardDate[30];
};

struct Transaction{
    int accountNo;
    char type[20];
    float amount;
    char otherAcc[30];
    char dateTime[30];
};


/* ===== Function Prototypes ===== */
void mainMenu();

/* Admin */
void adminLogin();
void createAdmin();
void adminPanel();
void createUser();
void changePin();
void deleteUser();
void manageATMCard();

/* ATM */
void atmLogin();
void atmMenu(struct User *u);
void deposit(struct User *u);
void withdraw(struct User *u);
void moneyTransfer(struct User *sender);
void balanceInquiry(struct User u);

/* User Panel */
void userLogin();
void userPanelMenu(struct User u);
void showATMCardInfo(struct User u);
void showTransactions(struct User u);

/* Utility */
void recordTransaction(struct User u, const char *type, float amount, const char *otherAcc);

/* ===== Helper Functions ===== */
void pauseScreen() {
    int c;
    printf("Press Enter to continue...");
    while ((c = getchar()) != '\n' && c != EOF);
}


int main(void) {
    mainMenu();
    return EXIT_SUCCESS;
}

void mainMenu(void) {
    int choice;
    int running = 1;

    while (running) {
        printf("\n=====================================\n");
        printf("\tDIGITAL BANKING SYSTEM\n");
        printf("\n=====================================\n");
        printf("\n1. ADMIN PANEL\n");
        printf("2. USER PANEL\n");
        printf("3. ATM\n");
        printf("4. EXIT\n");
        printf("\nEnter choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input! Please enter a number.\n");
            while (getchar() != '\n');   // clear buffer
            continue;
        }

        while (getchar() != '\n');       // clear extra input

        switch (choice) {
            case 1:
                adminLogin();
                break;

            case 2:
                userLogin();
                break;

            case 3:
                atmLogin();
                break;

            case 4: {
                int confirm;
                printf("Are you sure you want to exit? (1 = Yes, 0 = No): ");

                if (scanf("%d", &confirm) != 1) {
                    printf("Invalid input! Returning to menu.\n");
                    while (getchar() != '\n');
                    break;
                }

                while (getchar() != '\n');

                if (confirm == 1) {
                    running = 0;
                } else if (confirm != 0) {
                    printf("Invalid choice! Exit cancelled.\n");
                }
                break;
            }

            default:
                printf("Invalid choice! Please enter 1–4.\n");
        }
    }
}



/* ===== Admin Panel ===== */
void adminLogin() {
    int hasAccount;

    printf("\nDo you have admin account? (1=YES / 0=NO): ");
    if (scanf("%d", &hasAccount) != 1) {
        int c; while ((c = getchar()) != '\n' && c != EOF);
        printf("Invalid input! Returning to main menu.\n");
        return;
    }

    if (hasAccount != 0 && hasAccount != 1) {
        printf("Invalid choice! Only 0 or 1 allowed.\n");
        return;
    }

    if (hasAccount == 0) {
        createAdmin();
        return;
    }

    FILE *fp = fopen("admin.txt", "r");
    if (fp == NULL) {
        printf("No admin file found! Please create an admin account first.\n");
        return;
    }

    int inputId, inputCode;

    printf("Enter Admin ID: ");
    if (scanf("%d", &inputId) != 1) {
        int c; while ((c = getchar()) != '\n' && c != EOF);
        printf("Invalid Admin ID!\n");
        fclose(fp);
        return;
    }

    printf("Enter Code: ");
    if (scanf("%d", &inputCode) != 1) {
        int c; while ((c = getchar()) != '\n' && c != EOF);
        printf("Invalid Code!\n");
        fclose(fp);
        return;
    }

    char line[100];
    char username[30] = "";
    int id = 0, code = 0;
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "username : %29[^\n]", username) == 1) {

            if (fgets(line, sizeof(line), fp))
                sscanf(line, "id : %d", &id);

            if (fgets(line, sizeof(line), fp))
                sscanf(line, "code : %d", &code);

            /* optional separator skip */
            fgets(line, sizeof(line), fp);

            if (id == inputId && code == inputCode) {
                found = 1;
                break;
            }
        }
    }

    fclose(fp);

    if (found) {
        printf("Login successful. Welcome %s!\n", username);
        adminPanel();
    } else {
        printf("Wrong ID or Code!\n");
    }
}

void createAdmin() {
    FILE *fp = fopen("admin.txt", "a+");
    if (!fp) {
        printf("Error opening admin file!\n");
        return;
    }

    char username[30];
    int secretCode;
    int lastID = 1030000;   // starting base
    char line[100];

    /* ===== FIND LAST ADMIN ID ===== */
    rewind(fp);
    while (fgets(line, sizeof(line), fp)) {
        if (sscanf(line, "AdminID : %d", &lastID) == 1) {
            // keep updating lastID
        }
    }

    int newAdminID = lastID + 1;

    
    printf("\n--------------------------------\n");
    printf("\tCREATE ADMIN\n");
    printf("\n--------------------------------\n");

    printf("Enter Admin Name: ");
    scanf("%29s", username);

    printf("Enter Admin Secret Code: ");
    if (scanf("%d", &secretCode) != 1) {
        while (getchar() != '\n');
        printf("Invalid code!\n");
        fclose(fp);
        return;
    }

    
    time_t t = time(NULL);
    char dt[30];
    strcpy(dt, ctime(&t));
    dt[strcspn(dt, "\n")] = 0;

    fprintf(fp,
        "AdminName : %s\n"
        "AdminID   : %d\n"
        "Code      : %d\n"
        "DateTime  : %s\n"
        "----------------------\n",
        username, newAdminID, secretCode, dt
    );

    fclose(fp);

    
    printf("\nAdmin created successfully!\n");
    printf("Admin Name : %s\n", username);
    printf("Admin ID   : %d\n", newAdminID);
    printf("Date & Time: %s\n", dt);
}

void adminPanel() {
    int choice;

    while (1) {
        printf("\n--------------------------------\n");
        printf("\tADMIN PANEL");
        printf("\n--------------------------------\n");
        printf("1. CREATE USER ACCOUNT\n");
        printf("2. CHANGE USER PIN\n");
        printf("3. DELETE USER\n");
        printf("4. BLOCK / ACTIVATE ATM CARD\n");
        printf("5. BACK TO MAIN MENU\n");
        printf("Choice: ");

        if (scanf("%d", &choice) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("Invalid input!\n");
            continue;
        }

        switch (choice) {
            case 1: createUser(); break;
            case 2: changePin(); break;
            case 3: deleteUser(); break;
            case 4: manageATMCard(); break;
            case 5: return;
            default: printf("Invalid choice!\n");
        }
    }
}


void createUser() {
    FILE *fp = fopen("users.txt", "a");
    FILE *fpPanel = fopen("atmcard.txt", "a");
    if(!fp || !fpPanel){
        printf("Error opening files!\n");
        if(fp) fclose(fp);
        if(fpPanel) fclose(fpPanel);
        return;
    }

    struct User u;

    // ===== Auto-generate sequential account number =====
    int lastAcc = 12034002; // starting point
    FILE *fpCheck = fopen("users.txt", "r");
    if(fpCheck){
        struct User tempU;
        char label[20];
        while(fscanf(fpCheck,"%s : %d\n",label,&tempU.accountNo)!=EOF){
            fscanf(fpCheck,"%s : %[^\n]\n",label,tempU.name);
            fscanf(fpCheck,"%s : %d\n",label,&tempU.pin);
            fscanf(fpCheck,"%s : %f\n",label,&tempU.balance);
            fscanf(fpCheck,"%s\n",label); // separator
            if(tempU.accountNo > lastAcc) lastAcc = tempU.accountNo;
        }
        fclose(fpCheck);
    }
    u.accountNo = lastAcc + 1;

    // ===== Time handling =====
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(u.cardDate, sizeof(u.cardDate), "%a %b %d %H:%M:%S %Y", tm_info);

    // ===== Input =====
    printf("\n=== Create User ===\n");
    printf("User Name: ");
    scanf(" %29[^\n]", u.name); // allow spaces
    printf("4-digit PIN: ");
    scanf("%d", &u.pin);

    u.balance = 0.0;
    u.atmCardNo = 10000000 + u.accountNo;
    u.cardStatus = 1;

    // ===== Save to users.txt =====
    fprintf(fp,"account : %d\nname : %s\npin : %d\nbalance : %.2f\n------------------\n",
        u.accountNo, u.name, u.pin, u.balance);

    // ===== Save to atmcard.txt =====
    fprintf(fpPanel,
        "AccountNo: %d\nUserName: %s\nPin: %d\nAtmCard No: %ld\nCardStatus: %s\nDate and time: %s\n------------------\n",
        u.accountNo, u.name, u.pin, u.atmCardNo, "ACTIVE", u.cardDate);

    fclose(fp);
    fclose(fpPanel);

    printf("\nUser account created successfully!\n");

    // ===== Show ATM card on screen =====
    printf("\n--------------------------------\n");
    printf("\tATMCARD ISSED");
    printf("\n--------------------------------\n");
    printf("AccountNo: %d\n", u.accountNo);
    printf("UserName: %s\n", u.name);
    printf("AtmCard No: %ld\n", u.atmCardNo);
    printf("CardStatus: ACTIVE\n");
    printf("Date and time: %s\n", u.cardDate);

}


void changePin() {
    FILE *fp = fopen("users.txt", "r");
    FILE *temp = fopen("temp.txt", "w");

    if (!fp || !temp) {
        printf("Error opening files!\n");
        if (fp) fclose(fp);
        if (temp) fclose(temp);
        return;
    }

    int targetAcc, oldPin, newPin;

    printf("Enter Account No: ");
    if (scanf("%d", &targetAcc) != 1) goto invalid;

    printf("Enter CURRENT PIN: ");
    if (scanf("%d", &oldPin) != 1) goto invalid;

    printf("Enter New PIN (4 digits): ");
    if (scanf("%d", &newPin) != 1 || newPin < 1000 || newPin > 9999) {
        printf("Invalid new PIN!\n");
        goto cleanup;
    }

    struct User u;
    char line[200];
    int accountFound = 0, pinChanged = 0;

    while (fgets(line, sizeof(line), fp)) {
        sscanf(line, "account : %d", &u.accountNo);
        fgets(line, sizeof(line), fp); sscanf(line, "name : %29[^\n]", u.name);
        fgets(line, sizeof(line), fp); sscanf(line, "pin : %d", &u.pin);
        fgets(line, sizeof(line), fp); sscanf(line, "balance : %f", &u.balance);
        fgets(line, sizeof(line), fp); // separator

        if (u.accountNo == targetAcc) {
            accountFound = 1;
            if (u.pin == oldPin) {
                u.pin = newPin;
                pinChanged = 1;
            }
        }

        fprintf(temp,
            "account : %d\nname : %s\npin : %d\nbalance : %.2f\n------------------\n",
            u.accountNo, u.name, u.pin, u.balance);
    }

cleanup:
    fclose(fp);
    fclose(temp);

    if (!accountFound) {
        printf("Account not found!\n");
        remove("temp.txt");
        return;
    }
    if (!pinChanged) {
        printf("Incorrect current PIN!\n");
        remove("temp.txt");
        return;
    }

    remove("users.txt");
    rename("temp.txt", "users.txt");
    printf("PIN changed successfully!\n");
    return;

invalid:
    while (getchar() != '\n');
    printf("Invalid input!\n");
    goto cleanup;
}

void deleteUser() {
    int targetAcc, pin;

    printf("Enter Account No: ");
    if (scanf("%d", &targetAcc) != 1) goto invalid;

    printf("Enter PIN: ");
    if (scanf("%d", &pin) != 1) goto invalid;

    FILE *fp = fopen("users.txt", "r");
    FILE *temp = fopen("temp.txt", "w");
    if (!fp || !temp) {
        printf("File error!\n");
        if (fp) fclose(fp);
        if (temp) fclose(temp);
        return;
    }

    struct User u;
    char line[200];
    int accountFound = 0, pinMatched = 0;

    while (fgets(line, sizeof(line), fp)) {
        sscanf(line, "account : %d", &u.accountNo);
        fgets(line, sizeof(line), fp); sscanf(line, "name : %29[^\n]", u.name);
        fgets(line, sizeof(line), fp); sscanf(line, "pin : %d", &u.pin);
        fgets(line, sizeof(line), fp); sscanf(line, "balance : %f", &u.balance);
        fgets(line, sizeof(line), fp);

        if (u.accountNo == targetAcc) {
            accountFound = 1;
            if (u.pin == pin) {
                pinMatched = 1;
                continue;   // delete
            }
        }

        fprintf(temp,
            "account : %d\nname : %s\npin : %d\nbalance : %.2f\n------------------\n",
            u.accountNo, u.name, u.pin, u.balance);
    }

    fclose(fp);
    fclose(temp);

    if (!accountFound) {
        printf("Account not found!\n");
        remove("temp.txt");
        return;
    }
    if (!pinMatched) {
        printf("Incorrect PIN!\n");
        remove("temp.txt");
        return;
    }

    remove("users.txt");
    rename("temp.txt", "users.txt");

    /* ATM CARD DELETE */
    FILE *fpC = fopen("atmcard.txt", "r");
    FILE *tempC = fopen("tempCard.txt", "w");

    char buffer[200];
    int acc;

    while (fgets(buffer, sizeof(buffer), fpC)) {
        sscanf(buffer, "AccountNo: %d", &acc);

        char name[50], pinStr[10], status[15], date[50];
        long card;

        fgets(buffer, sizeof(buffer), fpC); sscanf(buffer, "UserName: %49[^\n]", name);
        fgets(buffer, sizeof(buffer), fpC); sscanf(buffer, "Pin: %9[^\n]", pinStr);
        fgets(buffer, sizeof(buffer), fpC); sscanf(buffer, "AtmCard No: %ld", &card);
        fgets(buffer, sizeof(buffer), fpC); sscanf(buffer, "CardStatus: %14[^\n]", status);
        fgets(buffer, sizeof(buffer), fpC); sscanf(buffer, "Date and time: %49[^\n]", date);
        fgets(buffer, sizeof(buffer), fpC);

        if (acc == targetAcc) continue;

        fprintf(tempC,
            "AccountNo: %d\nUserName: %s\nPin: %s\nAtmCard No: %ld\nCardStatus: %s\nDate and time: %s\n------------------\n",
            acc, name, pinStr, card, status, date);
    }

    fclose(fpC);
    fclose(tempC);

    remove("atmcard.txt");
    rename("tempCard.txt", "atmcard.txt");

    printf("User and ATM Card deleted successfully!\n");
    return;

invalid:
    while (getchar() != '\n');
    printf("Invalid input!\n");
}


/* ===== Manage ATM Card ===== */
void manageATMCard() {
    FILE *fp = fopen("atmcard.txt", "r");
    FILE *temp = fopen("temp.txt", "w");
    if (!fp || !temp) {
        printf("File error!\n");
        if (fp) fclose(fp);
        if (temp) fclose(temp);
        return;
    }

    int acc, choice;
    long card;
    char line[200];
    int found = 0;

    printf("Enter Account No: ");
    if (scanf("%d", &acc) != 1) goto invalid;

    printf("Enter ATM Card No: ");
    if (scanf("%ld", &card) != 1) goto invalid;

    printf("1. BLOCK CARD\n2. ACTIVATE CARD\nChoice: ");
    if (scanf("%d", &choice) != 1 || (choice != 1 && choice != 2)) goto invalid;

    while (fgets(line, sizeof(line), fp)) {
        int fileAcc;
        char userName[50], pin[20], status[15], date[50];
        long fileCard;

        sscanf(line, "AccountNo: %d", &fileAcc);
        fgets(line, sizeof(line), fp); sscanf(line, "UserName: %49[^\n]", userName);
        fgets(line, sizeof(line), fp); sscanf(line, "Pin: %19[^\n]", pin);
        fgets(line, sizeof(line), fp); sscanf(line, "AtmCard No: %ld", &fileCard);
        fgets(line, sizeof(line), fp); sscanf(line, "CardStatus: %14[^\n]", status);
        fgets(line, sizeof(line), fp); sscanf(line, "Date and time: %49[^\n]", date);
        fgets(line, sizeof(line), fp);

        if (fileAcc == acc && fileCard == card) {
            strcpy(status, choice == 1 ? "BLOCKED" : "ACTIVE");
            found = 1;
        }

        fprintf(temp,
            "AccountNo: %d\nUserName: %s\nPin: %s\nAtmCard No: %ld\nCardStatus: %s\nDate and time: %s\n------------------\n",
            fileAcc, userName, pin, fileCard, status, date);
    }

    fclose(fp);
    fclose(temp);

    remove("atmcard.txt");
    rename("temp.txt", "atmcard.txt");

    printf(found ? "ATM Card status updated successfully!\n" : "Account/Card not found!\n");
    return;

invalid:
    while (getchar() != '\n');
    printf("Invalid input!\n");
}


void atmMenu(struct User *u) {
    int choice;
    while(1){
        printf("\n--------------------------------\n");
        printf("\tATM");
        printf("\n--------------------------------\n");
        printf("1. DEPOSIT\n2. WITHDRAW\n3. BALANCE INQUIRY\n4. MONEY TRANSFER\n5. LOGOUT\n");
        printf("Choice: ");

        if(scanf("%d",&choice)!=1){
            while(getchar()!='\n'); // clear invalid input
            printf("Invalid input! Please enter a number.\n");
            continue;
        }

        switch(choice){
            case 1: 
                deposit(u);          // pointer
                break;
            case 2: 
                withdraw(u);         // pointer
                break;
            case 3: 
                balanceInquiry(*u);  // struct
                break;
            case 4: 
                moneyTransfer(u);    // pointer
                break;
            case 5: 
                return;
            default: 
                printf("Invalid choice! Please enter 1-5.\n");
        }
    }
}


/* ===== Transaction ===== */
void recordTransaction(struct User u, const char* type, float amount, const char* otherAcc){
    FILE *fp = fopen("transactions.txt","a");
    if(!fp){
        printf("Error opening transactions file!\n");
        return;
    }

    time_t t = time(NULL);
    char dt[30];
    strcpy(dt, ctime(&t));
    dt[strcspn(dt,"\n")] = 0;

    fprintf(fp,
        "AccountNo        : %d\n"
        "Transaction Type : %s\n"
        "Amount           : %.2f\n"
        "Other Account    : %s\n"
        "Date & Time      : %s\n"
        "----------------------------\n",
        u.accountNo, type, amount, otherAcc ? otherAcc : "-", dt
    );

    fclose(fp);
}

void atmLogin() {
    FILE *fp = fopen("atmcard.txt", "r");
    if (!fp) {
        printf("No users found!\n");
        return;
    }

    int inputAcc, inputPin;
    printf("Enter Account No: ");
    if (scanf("%d", &inputAcc) != 1) { while(getchar()!='\n'); fclose(fp); return; }

    printf("Enter PIN: ");
    if (scanf("%d", &inputPin) != 1) { while(getchar()!='\n'); fclose(fp); return; }

    char line[200];
    int found = 0;
    struct User u;

    while (fgets(line, sizeof(line), fp)) {
        int acc;
        char userName[50], pinStr[10], cardStatus[15], dateTime[50];
        long cardNo;

        sscanf(line, "AccountNo: %d", &acc);
        fgets(line, sizeof(line), fp); sscanf(line, "UserName: %49[^\n]", userName);
        fgets(line, sizeof(line), fp); sscanf(line, "Pin: %9[^\n]", pinStr);
        fgets(line, sizeof(line), fp); sscanf(line, "AtmCard No: %ld", &cardNo);
        fgets(line, sizeof(line), fp); sscanf(line, "CardStatus: %14[^\n]", cardStatus);
        fgets(line, sizeof(line), fp); sscanf(line, "Date and time: %49[^\n]", dateTime);
        fgets(line, sizeof(line), fp); // separator

        int filePin;
        sscanf(pinStr, "%d", &filePin);

        if (acc == inputAcc && filePin == inputPin) {
            u.accountNo = acc;
            strcpy(u.name, userName);
            u.pin = filePin;
            u.atmCardNo = cardNo;
            strcpy(u.cardDate, dateTime);
            u.cardStatus = (strcmp(cardStatus, "ACTIVE") == 0) ? 1 : 0;
            found = 1;
            break;
        }
    }

    fclose(fp);

    if (!found) {
        printf("Invalid account or PIN!\n");
        return;
    }

    if (!u.cardStatus) {
        printf("Your ATM card is BLOCKED! Contact admin.\n");
        return;
    }

    // Pass pointer to atmMenu
    atmMenu(&u);
}


/* ===== Deposit, Withdraw, Money Transfer, Balance Inquiry ===== */
// Deposit function safer version
/* ===== Deposit ===== */
void deposit(struct User *u){
    float amt;
    printf("Enter amount to deposit: ");
    if(scanf("%f",&amt)!=1 || amt <= 0){
        while(getchar()!='\n'); // clear invalid input
        printf("Invalid amount!\n");
        return;
    }

    FILE *fp=fopen("users.txt","r");
    FILE *temp=fopen("temp.txt","w");
    if(!fp||!temp){ printf("File error!\n"); if(fp) fclose(fp); if(temp) fclose(temp); return; }

    struct User tempUser;
    char label[20];
    while(fscanf(fp,"%s : %d\n",label,&tempUser.accountNo)!=EOF){
        fscanf(fp,"%s : %s\n",label,tempUser.name);
        fscanf(fp,"%s : %d\n",label,&tempUser.pin);
        fscanf(fp,"%s : %f\n",label,&tempUser.balance);
        fscanf(fp,"%s\n",label);

        if(tempUser.accountNo==u->accountNo){
            tempUser.balance += amt;
            u->balance += amt;  // update runtime struct
        }

        fprintf(temp,"account : %d\nname : %s\npin : %d\nbalance : %.2f\n------------------\n",
            tempUser.accountNo,tempUser.name,tempUser.pin,tempUser.balance);
    }

    fclose(fp);
    fclose(temp);
    remove("users.txt");
    rename("temp.txt","users.txt");

    printf("Deposit Successful! New Balance: %.2f\n", u->balance);
    recordTransaction(*u,"Deposit",amt,"-");  // <-- pass struct User, not pointer
}

/* ===== Withdraw ===== */
void withdraw(struct User *u){
    float amt;
    printf("Enter amount to withdraw: ");
    if(scanf("%f",&amt)!=1 || amt <= 0){
        while(getchar()!='\n');
        printf("Invalid amount!\n");
        return;
    }
    if(amt>u->balance){ printf("Insufficient Balance!\n"); return; }

    FILE *fp=fopen("users.txt","r");
    FILE *temp=fopen("temp.txt","w");
    if(!fp || !temp){ printf("File error!\n"); if(fp) fclose(fp); if(temp) fclose(temp); return; }

    struct User tempUser;
    char label[20];
    while(fscanf(fp,"%s : %d\n",label,&tempUser.accountNo)!=EOF){
        fscanf(fp,"%s : %s\n",label,tempUser.name);
        fscanf(fp,"%s : %d\n",label,&tempUser.pin);
        fscanf(fp,"%s : %f\n",label,&tempUser.balance);
        fscanf(fp,"%s\n",label);

        if(tempUser.accountNo==u->accountNo) tempUser.balance-=amt;

        fprintf(temp,"account : %d\nname : %s\npin : %d\nbalance : %.2f\n------------------\n",
            tempUser.accountNo,tempUser.name,tempUser.pin,tempUser.balance);
    }

    fclose(fp);
    fclose(temp);
    remove("users.txt");
    rename("temp.txt","users.txt");

    u->balance -= amt;
    printf("Withdraw Successful! New Balance: %.2f\n", u->balance);
    recordTransaction(*u,"Withdraw",amt,"-"); // <-- fixed
}

/* ===== Money Transfer ===== */
void moneyTransfer(struct User *sender){
    int targetAcc;
    float amount;

    printf("Enter Receiver Account No: ");
    if(scanf("%d",&targetAcc)!=1){ while(getchar()!='\n'); printf("Invalid input!\n"); return; }

    printf("Enter Amount to Transfer: ");
    if(scanf("%f",&amount)!=1 || amount<=0){ while(getchar()!='\n'); printf("Invalid amount!\n"); return; }

    if(amount > sender->balance){
        printf("Your balance is not sufficient!\n");
        return;
    }

    FILE *fp = fopen("users.txt","r");
    FILE *temp = fopen("temp.txt","w");
    if(!fp || !temp){ printf("File error!\n"); if(fp) fclose(fp); if(temp) fclose(temp); return; }

    struct User tempUser;
    char label[20];
    int receiverFound = 0;
    int senderFound = 0;

    /* FIRST PASS: check receiver exists */
    while(fscanf(fp,"%s : %d\n",label,&tempUser.accountNo)!=EOF){
        fscanf(fp,"%s : %s\n",label,tempUser.name);
        fscanf(fp,"%s : %d\n",label,&tempUser.pin);
        fscanf(fp,"%s : %f\n",label,&tempUser.balance);
        fscanf(fp,"%s\n",label);

        if(tempUser.accountNo == targetAcc){
            receiverFound = 1;
            break;
        }
    }

    if(!receiverFound){
        printf("Receiver account not found!\n");
        fclose(fp);
        fclose(temp);
        remove("temp.txt");
        return;
    }

    rewind(fp);

    /* SECOND PASS: apply transfer */
    while(fscanf(fp,"%s : %d\n",label,&tempUser.accountNo)!=EOF){
        fscanf(fp,"%s : %s\n",label,tempUser.name);
        fscanf(fp,"%s : %d\n",label,&tempUser.pin);
        fscanf(fp,"%s : %f\n",label,&tempUser.balance);
        fscanf(fp,"%s\n",label);

        if(tempUser.accountNo == sender->accountNo){
            tempUser.balance -= amount;
            senderFound = 1;
        }
        else if(tempUser.accountNo == targetAcc){
            tempUser.balance += amount;
        }

        fprintf(temp,
            "account : %d\nname : %s\npin : %d\nbalance : %.2f\n------------------\n",
            tempUser.accountNo,tempUser.name,tempUser.pin,tempUser.balance);
    }

    fclose(fp);
    fclose(temp);

    if(!senderFound){
        printf("Sender account error!\n");
        remove("temp.txt");
        return;
    }

    remove("users.txt");
    rename("temp.txt","users.txt");

    char otherAcc[20];
    sprintf(otherAcc,"%d",targetAcc);

    printf("Money transferred successfully!\n");
    recordTransaction(*sender,"Transfer",amount,otherAcc); // <-- fixed
}


void balanceInquiry(struct User u){
    FILE *fp=fopen("users.txt","r");
    if(!fp){ printf("Error opening file!\n"); return; }

    struct User tempUser; char label[20];
    while(fscanf(fp,"%s : %d\n",label,&tempUser.accountNo)!=EOF){
        fscanf(fp,"%s : %s\n",label,tempUser.name);
        fscanf(fp,"%s : %d\n",label,&tempUser.pin);
        fscanf(fp,"%s : %f\n",label,&tempUser.balance);
        fscanf(fp,"%s\n",label);

        if(tempUser.accountNo==u.accountNo){ printf("Current Balance: %.2f\n",tempUser.balance); break; }
    }
    fclose(fp);
}

/* ===== User Panel ===== */
void userLogin(){
    FILE *fp = fopen("atmcard.txt","r");
    if(!fp){
        printf("No users found!\n");
        return;
    }

    int inputAcc, inputPin;
    printf("Enter Account No: ");
    if(scanf("%d",&inputAcc)!=1){ while(getchar()!='\n'); fclose(fp); return; }

    printf("Enter PIN: ");
    if(scanf("%d",&inputPin)!=1){ while(getchar()!='\n'); fclose(fp); return; }

    char line[200];
    int found = 0;
    struct User u;

    while(fgets(line,sizeof(line),fp)){
        int acc;
        char userName[50], pinStr[10], cardStatus[15], dateTime[50];
        long cardNo;

        sscanf(line,"AccountNo: %d",&acc);
        fgets(line,sizeof(line),fp); sscanf(line,"UserName: %49[^\n]",userName);
        fgets(line,sizeof(line),fp); sscanf(line,"Pin: %9[^\n]",pinStr);
        fgets(line,sizeof(line),fp); sscanf(line,"AtmCard No: %ld",&cardNo);
        fgets(line,sizeof(line),fp); sscanf(line,"CardStatus: %14[^\n]",cardStatus);
        fgets(line,sizeof(line),fp); sscanf(line,"Date and time: %49[^\n]",dateTime);
        fgets(line,sizeof(line),fp); // separator

        int filePin;
        sscanf(pinStr,"%d",&filePin);

        if(acc == inputAcc && filePin == inputPin){
            u.accountNo = acc;
            strcpy(u.name,userName);
            u.pin = filePin;
            u.atmCardNo = cardNo;
            strcpy(u.cardDate,dateTime);
            u.cardStatus = (strcmp(cardStatus,"ACTIVE") == 0);
            found = 1;
            break;
}

    }

    fclose(fp);

    if(found) userPanelMenu(u);
    else printf("Invalid account or PIN!\n");
}

void userPanelMenu(struct User u){
    int choice;
    while(1){
        printf("\n--------------------------------\n");
        printf("\tUSER PANEL");
        printf("\n--------------------------------\n");
        printf("1. Display ATM Card Info\n2. Show Transactions\n3. Back to Main Menu\nEnter choice: ");
        
        if(scanf("%d",&choice)!=1){ while(getchar()!='\n'); continue; }

        switch(choice){
            case 1: showATMCardInfo(u); break;
            case 2: showTransactions(u); break;
            case 3: return;
            default: printf("Invalid choice!\n");
        }
    }
}
void showATMCardInfo(struct User u){
    printf("\n--------------------------------\n");
    printf("\tATM CARD INFO");
    printf("\n--------------------------------\n");
    printf("AccountNo: %d\n", u.accountNo);
    printf("UserName: %s\n", u.name);
    printf("Pin: %d\n", u.pin);
    printf("AtmCard No: %ld\n", u.atmCardNo);
    printf("Date and time: %s\n", u.cardDate);
    if(!u.cardStatus) 
        printf("WARNING: Card is BLOCKED!\n");
}


void showTransactions(struct User u){
    FILE *fp = fopen("transactions.txt","r");
    if(!fp){ 
        printf("No transactions found!\n"); 
        return; 
    }

    struct Transaction t;
    char line[200];
    printf("\n--------------------------------\n");
    printf("\tYour Transactions");
    printf("\n--------------------------------\n");
    
    while(fgets(line,sizeof(line),fp)){
        if(sscanf(line,"AccountNo        : %d",&t.accountNo)==1){
            fgets(line,sizeof(line),fp); sscanf(line,"Transaction Type : %19[^\n]",t.type);
            fgets(line,sizeof(line),fp); sscanf(line,"Amount           : %f",&t.amount);
            fgets(line,sizeof(line),fp); sscanf(line,"Other Account    : %29[^\n]",t.otherAcc);
            fgets(line,sizeof(line),fp); sscanf(line,"Date & Time      : %29[^\n]",t.dateTime);
            fgets(line,sizeof(line),fp); // separator

            if(t.accountNo==u.accountNo){
                printf("Transaction Type: %s\n", t.type);
                printf("Amount         : %.2f\n", t.amount);
                printf("Other Account  : %s\n", t.otherAcc);
                printf("Date & Time    : %s\n", t.dateTime);
                printf("---------------------------------------\n");
            }
        }
    }
    fclose(fp);
}   
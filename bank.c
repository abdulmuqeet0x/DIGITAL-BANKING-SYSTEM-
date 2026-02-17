#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "user.h"   // shared header for User struct, file defines, macros

#define TEMP_FILE "temp_users.txt"

/* ================================================
   UTILITY FUNCTIONS
   ================================================ */
void getCurrentDateTime(char str[]) {
    time_t t;
    struct tm *tm_info;
    time(&t);
    tm_info = localtime(&t);
    strftime(str, 30, "%d-%m-%Y %H:%M:%S", tm_info);
}

void auditLog(char message[]) {
    FILE *fp = fopen(AUDIT_LOG_FILE, "a");  // FIX: "a" not "rb"
    if (!fp) return;
    char dateTime[30];
    getCurrentDateTime(dateTime);
    fprintf(fp, "[%s] %s\n", dateTime, message);
    fclose(fp);
}

/* ================================================
   INBOX STRUCT (local to bank.c)
   ================================================ */
typedef struct {
    long long accountNumber;   // FIX: was int, now long long
    char message[200];
    char dateTime[30];
} Inbox;

/* ================================================
   TRANSACTION STRUCT (local to bank.c)
   ================================================ */
typedef struct {
    long long accountNumber;         // FIX: was int
    char      type[20];
    double    amount;
    long long otherAccountNumber;    // FIX: was int
    char      dateTime[30];
} Transaction;

/* ================================================
   READ / WRITE USER FUNCTIONS
   ================================================ */
int readUser(long long accountNumber, User *user) {
    FILE *fp = fopen(USER_FILE, "rb");
    if (!fp) return 0;
    while (fread(user, sizeof(User), 1, fp)) {
        if (user->accountNo == accountNumber) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int readUserByAtmCard(long long atmCardNumber, User *user) {
    FILE *fp = fopen(USER_FILE, "rb");
    if (!fp) return 0;
    while (fread(user, sizeof(User), 1, fp)) {
        if (user->atmCardNo == atmCardNumber) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int updateUser(long long accountNumber, User updatedUser) {
    FILE *fp     = fopen(USER_FILE, "rb");
    FILE *tempFp = fopen(TEMP_FILE, "wb");

    if (fp == NULL || tempFp == NULL) {
        printf("Error opening user file.\n");
        if (fp)    fclose(fp);
        if (tempFp) fclose(tempFp);
        return 0;
    }

    User temp;
    int found = 0;
    while (fread(&temp, sizeof(User), 1, fp)) {
        if (temp.accountNo == accountNumber) {
            fwrite(&updatedUser, sizeof(User), 1, tempFp);
            found = 1;
        } else {
            fwrite(&temp, sizeof(User), 1, tempFp);
        }
    }

    fclose(fp);
    fclose(tempFp);

    if (found) {
        remove(USER_FILE);
        rename(TEMP_FILE, USER_FILE);
    } else {
        remove(TEMP_FILE);
    }
    return found;
}

int deleteUser(long long accountNumber) {
    FILE *fp     = fopen(USER_FILE, "rb");
    FILE *tempFp = fopen(TEMP_FILE, "wb");

    if (fp == NULL || tempFp == NULL) {
        printf("Error opening user file.\n");
        if (fp)    fclose(fp);
        if (tempFp) fclose(tempFp);
        return 0;
    }

    User temp;
    int found = 0;
    while (fread(&temp, sizeof(User), 1, fp)) {
        if (temp.accountNo == accountNumber) {
            found = 1;
            continue;
        }
        fwrite(&temp, sizeof(User), 1, tempFp);
    }

    fclose(fp);
    fclose(tempFp);

    if (found) {
        remove(USER_FILE);
        rename(TEMP_FILE, USER_FILE);
    } else {
        remove(TEMP_FILE);
    }
    return found;
}

void addUser(User newUser) {
    FILE *fp;
    long long lastAccountNumber = 1000;
    long long lastAtmCardNumber = 50000;

    fp = fopen(USER_FILE, "rb");
    if (fp != NULL) {
        User temp;
        while (fread(&temp, sizeof(User), 1, fp)) {
            if (temp.accountNo > lastAccountNumber)
                lastAccountNumber = temp.accountNo;
            if (temp.atmCardNo > lastAtmCardNumber)
                lastAtmCardNumber = temp.atmCardNo;
        }
        fclose(fp);
    }

    newUser.accountNo = lastAccountNumber + 1;
    newUser.atmCardNo = lastAtmCardNumber + 1;
    strcpy(newUser.accountStatus, "Active");
    strcpy(newUser.cardStatus,    "Active");
    newUser.balance = 0.0;
    getCurrentDateTime(newUser.dateTime);

    fp = fopen(USER_FILE, "ab");
    if (fp == NULL) {
        printf("Error opening user file.\n");
        return;
    }
    fwrite(&newUser, sizeof(User), 1, fp);
    fclose(fp);

    printf("User created successfully!\n");
    printf("Account Number : %lld\n", newUser.accountNo);
    printf("ATM Card Number: %lld\n", newUser.atmCardNo);
}

/* ================================================
   INBOX FUNCTIONS
   ================================================ */
void sendUserInboxMessage(long long accountNumber, const char *message) {
    Inbox inboxMsg;
    inboxMsg.accountNumber = accountNumber;
    strncpy(inboxMsg.message, message, sizeof(inboxMsg.message) - 1);
    inboxMsg.message[sizeof(inboxMsg.message) - 1] = '\0';
    getCurrentDateTime(inboxMsg.dateTime);

    FILE *fp = fopen(USER_INBOX_FILE, "ab");
    if (!fp) { printf("Error opening user inbox file.\n"); return; }
    fwrite(&inboxMsg, sizeof(Inbox), 1, fp);
    fclose(fp);
}

void sendAdminInboxMessage(const char *message) {
    Inbox inboxMsg;
    inboxMsg.accountNumber = 0;
    strncpy(inboxMsg.message, message, sizeof(inboxMsg.message) - 1);
    inboxMsg.message[sizeof(inboxMsg.message) - 1] = '\0';
    getCurrentDateTime(inboxMsg.dateTime);

    FILE *fp = fopen(ADMIN_INBOX_FILE, "ab");
    if (!fp) { printf("Error opening admin inbox file.\n"); return; }
    fwrite(&inboxMsg, sizeof(Inbox), 1, fp);
    fclose(fp);
}

void viewUserInbox(long long accountNumber) {
    FILE *fp = fopen(USER_INBOX_FILE, "rb");
    if (!fp) { printf("No messages found.\n"); return; }

    Inbox inboxMsg;
    int found = 0;
    printf("\n--- Inbox ---\n");
    while (fread(&inboxMsg, sizeof(Inbox), 1, fp)) {
        if (inboxMsg.accountNumber == accountNumber) {
            printf("[%s] %s\n", inboxMsg.dateTime, inboxMsg.message);
            found = 1;
        }
    }
    fclose(fp);
    if (!found) printf("No messages in inbox.\n");
}

void viewAdminInbox() {
    FILE *fp = fopen(ADMIN_INBOX_FILE, "rb");
    if (!fp) { printf("No messages found.\n"); return; }

    Inbox msg;
    printf("\n--- Admin Inbox ---\n");
    int found = 0;
    while (fread(&msg, sizeof(Inbox), 1, fp)) {
        printf("[%s] %s\n", msg.dateTime, msg.message);
        found = 1;
    }
    fclose(fp);
    if (!found) printf("No messages in admin inbox.\n");
}

/* ================================================
   TRANSACTION FUNCTIONS
   ================================================ */
void addTransaction(Transaction trans) {
    FILE *fp = fopen(TRANSACTION_FILE, "ab");
    if (!fp) { printf("Error opening transaction file.\n"); return; }
    fwrite(&trans, sizeof(Transaction), 1, fp);
    fclose(fp);
}

void viewUserTransactions(long long accountNumber) {
    FILE *fp = fopen(TRANSACTION_FILE, "rb");
    if (!fp) { printf("No transactions found.\n"); return; }

    Transaction trans;
    int found = 0;
    printf("\n--- Transaction History ---\n");
    while (fread(&trans, sizeof(Transaction), 1, fp)) {
        if (trans.accountNumber == accountNumber) {
            if (strcmp(trans.type, "Deposit") == 0 || strcmp(trans.type, "Withdraw") == 0)
                printf("[%s] %s: %.2lf PKR\n", trans.dateTime, trans.type, trans.amount);
            else
                printf("[%s] %s: %.2lf PKR (Other Account: %lld)\n",
                       trans.dateTime, trans.type, trans.amount, trans.otherAccountNumber);
            found = 1;
        }
    }
    fclose(fp);
    if (!found) printf("No transactions found.\n");
}

void viewUserTransactionsUI() {
    char input[20];
    long long acc;
    printf("Enter account number to view transactions: ");
    fgets(input, sizeof(input), stdin);
    if (sscanf(input, "%lld", &acc) != 1) {
        printf("Invalid account number!\n");
        return;
    }
    viewUserTransactions(acc);
}

/* ================================================
   MINI STATEMENT
   ================================================ */
void miniStatement(long long accountNumber) {
    FILE *fp = fopen(TRANSACTION_FILE, "rb");
    if (!fp) { printf("No transactions found.\n"); return; }

    // Collect last 5 transactions
    Transaction all[10000];
    int total = 0;
    Transaction trans;
    while (fread(&trans, sizeof(Transaction), 1, fp)) {
        if (trans.accountNumber == accountNumber) {
            all[total++] = trans;
        }
    }
    fclose(fp);

    if (total == 0) { printf("No transactions found.\n"); return; }

    int start = (total > 5) ? total - 5 : 0;
    printf("\n--- Mini Statement (Last %d transactions) ---\n", total - start);
    for (int i = start; i < total; i++) {
        if (strcmp(all[i].type, "Deposit") == 0 || strcmp(all[i].type, "Withdraw") == 0)
            printf("[%s] %s: %.2lf PKR\n", all[i].dateTime, all[i].type, all[i].amount);
        else
            printf("[%s] %s: %.2lf PKR (Other: %lld)\n",
                   all[i].dateTime, all[i].type, all[i].amount, all[i].otherAccountNumber);
    }
}

/* ================================================
   BALANCE / DEPOSIT / WITHDRAW / TRANSFER
   ================================================ */
void viewBalance(long long accountNumber) {
    User user;
    if (!readUser(accountNumber, &user)) { printf("User not found!\n"); return; }
    printf("Your Current Balance: %.2lf PKR\n", user.balance);

    char msg[200];
    snprintf(msg, sizeof(msg), "Checked balance: %.2lf PKR", user.balance);
    sendUserInboxMessage(accountNumber, msg);
}

void checkBalance(User user) {
    printf("\n--- Account Balance ---\n");
    printf("Account Number : %lld\n", user.accountNo);
    printf("Name           : %s\n",   user.name);
    printf("Balance        : %.2lf PKR\n", user.balance);
    printf("----------------------\n");
}

void depositMoney(User *user) {
    double amount;
    char input[20];
    printf("Enter amount to deposit: ");
    fgets(input, sizeof(input), stdin);
    if (sscanf(input, "%lf", &amount) != 1 || amount <= 0) {
        printf("Invalid amount!\n");
        return;
    }

    double prev = user->balance;
    user->balance += amount;
    updateUser(user->accountNo, *user);

    Transaction trans = {0};
    trans.accountNumber = user->accountNo;
    strcpy(trans.type, "Deposit");
    trans.amount = amount;
    trans.otherAccountNumber = 0;
    getCurrentDateTime(trans.dateTime);
    addTransaction(trans);

    char msg[200];
    snprintf(msg, sizeof(msg), "Deposited %.2lf PKR. Prev: %.2lf, New: %.2lf", amount, prev, user->balance);
    sendUserInboxMessage(user->accountNo, msg);

    printf("\n--- Deposit Receipt ---\n");
    printf("Amount Deposited : %.2lf PKR\n", amount);
    printf("Previous Balance : %.2lf PKR\n", prev);
    printf("Current Balance  : %.2lf PKR\n", user->balance);
    printf("Date/Time        : %s\n", trans.dateTime);
    printf("-----------------------\n");
}

void depositAmount(long long accountNumber) {
    User user;
    if (!readUser(accountNumber, &user)) { printf("User not found!\n"); return; }
    depositMoney(&user);
}

void withdrawMoney(User *user) {
    double amount;
    char input[20];
    printf("Enter amount to withdraw: ");
    fgets(input, sizeof(input), stdin);
    if (sscanf(input, "%lf", &amount) != 1 || amount <= 0) {
        printf("Invalid amount!\n");
        return;
    }

    if (amount > user->balance) {
        printf("Insufficient balance! Current: %.2lf PKR\n", user->balance);
        return;
    }

    double prev = user->balance;
    user->balance -= amount;
    updateUser(user->accountNo, *user);

    Transaction trans = {0};
    trans.accountNumber = user->accountNo;
    strcpy(trans.type, "Withdraw");
    trans.amount = amount;
    trans.otherAccountNumber = 0;
    getCurrentDateTime(trans.dateTime);
    addTransaction(trans);

    char msg[200];
    snprintf(msg, sizeof(msg), "Withdrawn %.2lf PKR. Prev: %.2lf, New: %.2lf", amount, prev, user->balance);
    sendUserInboxMessage(user->accountNo, msg);

    printf("\n--- Withdrawal Receipt ---\n");
    printf("Amount Withdrawn : %.2lf PKR\n", amount);
    printf("Previous Balance : %.2lf PKR\n", prev);
    printf("Current Balance  : %.2lf PKR\n", user->balance);
    printf("Date/Time        : %s\n", trans.dateTime);
    printf("--------------------------\n");
}

void withdrawAmount(long long accountNumber) {
    User user;
    if (!readUser(accountNumber, &user)) { printf("User not found!\n"); return; }
    withdrawMoney(&user);
}

void transferMoney(long long senderAccountNumber) {
    User sender, receiver;
    char input[20];
    double amount;

    if (!readUser(senderAccountNumber, &sender)) { printf("Sender not found!\n"); return; }

    if (strcmp(sender.accountStatus, "Blocked") == 0 ||
        strcmp(sender.cardStatus,    "Blocked") == 0) {
        printf("Your account/card is blocked! Cannot transfer.\n");
        return;
    }

    long long receiverAccountNumber;
    while (1) {
        printf("Enter receiver's account number: ");
        fgets(input, sizeof(input), stdin);
        if (sscanf(input, "%lld", &receiverAccountNumber) != 1) {
            printf("Invalid input!\n"); continue;
        }
        if (receiverAccountNumber == senderAccountNumber) {
            printf("Cannot transfer to your own account!\n"); continue;
        }
        break;
    }

    if (!readUser(receiverAccountNumber, &receiver)) { printf("Receiver not found!\n"); return; }

    if (strcmp(receiver.accountStatus, "Blocked") == 0 ||
        strcmp(receiver.cardStatus,    "Blocked") == 0) {
        printf("Receiver's account/card is blocked!\n");
        return;
    }

    while (1) {
        printf("Enter amount to transfer: ");
        fgets(input, sizeof(input), stdin);
        if (sscanf(input, "%lf", &amount) != 1 || amount <= 0) {
            printf("Invalid amount!\n"); continue;
        }
        if (amount > sender.balance) {
            printf("Insufficient balance! Current: %.2lf PKR\n", sender.balance);
            continue;
        }
        break;
    }

    char dateTime[30];
    getCurrentDateTime(dateTime);

    double senderPrev   = sender.balance;
    double receiverPrev = receiver.balance;

    sender.balance   -= amount;
    receiver.balance += amount;

    updateUser(senderAccountNumber,   sender);
    updateUser(receiverAccountNumber, receiver);

    Transaction t1 = {0};
    t1.accountNumber = senderAccountNumber;
    strcpy(t1.type, "Transfer Sent");
    t1.amount = amount;
    t1.otherAccountNumber = receiverAccountNumber;
    strcpy(t1.dateTime, dateTime);
    addTransaction(t1);

    Transaction t2 = {0};
    t2.accountNumber = receiverAccountNumber;
    strcpy(t2.type, "Transfer Received");
    t2.amount = amount;
    t2.otherAccountNumber = senderAccountNumber;
    strcpy(t2.dateTime, dateTime);
    addTransaction(t2);

    char msg[250];
    snprintf(msg, sizeof(msg), "Transferred %.2lf PKR to %lld. Prev: %.2lf, New: %.2lf",
             amount, receiverAccountNumber, senderPrev, sender.balance);
    sendUserInboxMessage(senderAccountNumber, msg);

    snprintf(msg, sizeof(msg), "Received %.2lf PKR from %lld. Prev: %.2lf, New: %.2lf",
             amount, senderAccountNumber, receiverPrev, receiver.balance);
    sendUserInboxMessage(receiverAccountNumber, msg);

    printf("\n--- Transfer Receipt ---\n");
    printf("Sender Account    : %lld\n", senderAccountNumber);
    printf("Receiver Account  : %lld\n", receiverAccountNumber);
    printf("Amount Transferred: %.2lf PKR\n", amount);
    printf("Sender Balance    : Prev %.2lf -> Now %.2lf\n", senderPrev, sender.balance);
    printf("Receiver Balance  : Prev %.2lf -> Now %.2lf\n", receiverPrev, receiver.balance);
    printf("Date/Time         : %s\n", dateTime);
    printf("------------------------\n");
}

void transferAmount(long long senderAccountNumber) {
    transferMoney(senderAccountNumber);
}

/* ================================================
   ATM LOGIN
   ================================================ */
int atmUserLogin(long long atmCardNumber) {
    User user;
    if (!readUserByAtmCard(atmCardNumber, &user)) {
        printf("ATM Card not found!\n");
        return 0;
    }

    if (strcmp(user.accountStatus, "Blocked") == 0 ||
        strcmp(user.cardStatus,    "Blocked") == 0) {
        printf("Account/Card is blocked!\n");
        return 0;
    }

    int attempts = 0;
    while (attempts < 3) {
        char pinStr[10];
        int pin;
        printf("Enter PIN: ");
        fgets(pinStr, sizeof(pinStr), stdin);

        // FIX: no attempts-- on invalid input (caused infinite loop)
        if (sscanf(pinStr, "%d", &pin) != 1) {
            printf("Invalid input! Numbers only.\n");
            continue;
        }

        if (pin == user.mpin) {
            printf("Login Successful!\n");
            updateUser(user.accountNo, user);
            sendUserInboxMessage(user.accountNo, "ATM login successful.");
            return 1;
        }

        attempts++;
        printf("Incorrect PIN! Attempts left: %d\n", 3 - attempts);

        if (attempts >= 3) {
            strcpy(user.accountStatus, "Blocked");
            strcpy(user.cardStatus,    "Blocked");
            getCurrentDateTime(user.dateTime);
            updateUser(user.accountNo, user);
            sendUserInboxMessage(user.accountNo,
                "Account & ATM card blocked due to 3 incorrect PIN attempts.");
            sendAdminInboxMessage(
                "Account & ATM card blocked due to 3 incorrect PIN attempts.");
            printf("Account & ATM card blocked!\n");
        } else {
            updateUser(user.accountNo, user);
        }
    }
    return 0;
}

int verifyUserPin(long long accountNo, int pin) {
    User user;
    if (!readUser(accountNo, &user)) return 0;
    return (user.mpin == pin) ? 1 : 0;
}

/* ================================================
   ATM CARD INFO
   ================================================ */
void viewAtmCardInfo(long long accountNumber) {
    User user;
    if (!readUser(accountNumber, &user)) { printf("User not found!\n"); return; }

    printf("\n--- ATM Card Info ---\n");
    printf("ATM Card Number : %lld\n", user.atmCardNo);
    printf("Card Status     : %s\n",   user.cardStatus);
    printf("Account Status  : %s\n",   user.accountStatus);
    printf("Account Created : %s\n",   user.dateTime);
}

/* ================================================
   VIEW ALL USERS
   ================================================ */
void viewAllUsers() {
    FILE *fp = fopen(USER_FILE, "rb");
    if (!fp) { printf("No users found.\n"); return; }

    User user;
    int count = 0;
    printf("\n--- All Users ---\n");
    while (fread(&user, sizeof(User), 1, fp)) {
        printf("Account: %lld | Name: %-20s | Balance: %10.2lf PKR | Status: %-8s | Card: %-8s\n",
               user.accountNo, user.name, user.balance,
               user.accountStatus, user.cardStatus);
        count++;
    }
    fclose(fp);
    if (count == 0) printf("No users found.\n");
    else printf("Total Users: %d\n", count);
}

/* ================================================
   VIEW BLOCKED USERS
   ================================================ */
void viewBlockedUsers() {
    FILE *fp = fopen(USER_FILE, "rb");
    if (!fp) { printf("No users found.\n"); return; }

    User user;
    int found = 0;
    printf("\n--- Blocked Users ---\n");
    while (fread(&user, sizeof(User), 1, fp)) {
        if (strcmp(user.accountStatus, "Blocked") == 0) {
            printf("Account: %lld | Name: %-20s | Card: %-8s | Balance: %.2lf PKR\n",
                   user.accountNo, user.name, user.cardStatus, user.balance);
            found = 1;
        }
    }
    fclose(fp);
    if (!found) printf("No blocked users found.\n");
}

/* ================================================
   VIEW AUDIT LOG
   ================================================ */
void viewAuditLog() {
    FILE *fp = fopen(AUDIT_LOG_FILE, "r");  // FIX: "r" not "rb"
    if (!fp) { printf("No audit log found.\n"); return; }

    char line[300];
    int count = 1;
    printf("\n--- Audit Log ---\n");
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;
        printf("%d. %s\n", count++, line);
    }
    fclose(fp);
}

/* ================================================
   SEARCH USER
   ================================================ */
void searchUser() {
    FILE *fp = fopen(USER_FILE, "rb");
    if (!fp) { printf("No users found.\n"); return; }

    char input[50];
    printf("Enter Account Number, Name, or ATM Card Number: ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;

    User user;
    int found = 0;
    long long numValue = 0;
    int numericInput = (sscanf(input, "%lld", &numValue) == 1);  // FIX: long long

    while (fread(&user, sizeof(User), 1, fp)) {
        int match = 0;
        if (numericInput) {
            if (user.accountNo == numValue || user.atmCardNo == numValue)
                match = 1;
        } else {
            if (strstr(user.name, input) != NULL)
                match = 1;
        }

        if (match) {
            printf("\n--- User Found ---\n");
            printf("Account  : %lld\n", user.accountNo);
            printf("Name     : %s\n",   user.name);
            printf("Balance  : %.2lf PKR\n", user.balance);
            printf("Status   : %s\n",   user.accountStatus);
            printf("ATM Card : %lld | Card Status: %s\n", user.atmCardNo, user.cardStatus);
            printf("Created  : %s\n",   user.dateTime);
            found = 1;
        }
    }
    fclose(fp);
    if (!found) printf("No matching user found.\n");
}

void searchUserUI() {
    searchUser();
}

/* ================================================
   EDIT USER INFO
   ================================================ */
void editUserInfo() {
    char input[50];
    long long acc;
    User user;

    printf("Enter account number to edit: ");
    fgets(input, sizeof(input), stdin);
    if (sscanf(input, "%lld", &acc) != 1) { printf("Invalid!\n"); return; }

    if (!readUser(acc, &user)) { printf("User not found!\n"); return; }

    printf("\n--- Current Info ---\n");
    printf("Name   : %s\n", user.name);
    printf("Balance: %.2lf PKR\n", user.balance);

    printf("Enter new name (ENTER to skip): ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;
    if (strlen(input) > 0) {
        strncpy(user.name, input, sizeof(user.name) - 1);
        user.name[sizeof(user.name) - 1] = '\0';
    }

    printf("Enter new 4-digit PIN (0 to skip): ");
    fgets(input, sizeof(input), stdin);
    int newPin;
    if (sscanf(input, "%d", &newPin) == 1 && newPin >= 1000 && newPin <= 9999)
        user.mpin = newPin;

    printf("Enter new balance (-1 to skip): ");
    fgets(input, sizeof(input), stdin);
    double newBalance;
    // FIX: use integer flag instead of comparing double == -1
    int skipBalance = 0;
    if (sscanf(input, "%lf", &newBalance) == 1) {
        if (newBalance < 0) skipBalance = 1;
        if (!skipBalance)   user.balance = newBalance;
    }

    getCurrentDateTime(user.dateTime);
    if (updateUser(acc, user)) {
        printf("User updated successfully.\n");
        sendUserInboxMessage(acc, "Admin updated your account info.");
        char log[200];
        snprintf(log, sizeof(log), "Admin edited user %lld info.", acc);
        auditLog(log);
    } else {
        printf("Error updating user.\n");
    }
}

void editUserInfoUI() {
    editUserInfo();
}

/* ================================================
   TOGGLE BLOCK / UNBLOCK
   ================================================ */
void toggleUserBlockStatus() {
    char input[20];
    long long acc;

    printf("Enter account number: ");
    fgets(input, sizeof(input), stdin);
    if (sscanf(input, "%lld", &acc) != 1) { printf("Invalid!\n"); return; }

    User user;
    if (!readUser(acc, &user)) { printf("User not found!\n"); return; }

    char msg[200];

    if (strcmp(user.accountStatus, "Active") == 0) {
        char reason[100];
        printf("Enter reason for blocking: ");
        fgets(reason, sizeof(reason), stdin);
        reason[strcspn(reason, "\n")] = 0;
        if (strlen(reason) == 0) strcpy(reason, "Blocked by admin");

        strcpy(user.accountStatus, "Blocked");
        strcpy(user.cardStatus,    "Blocked");
        getCurrentDateTime(user.dateTime);
        updateUser(acc, user);

        snprintf(msg, sizeof(msg), "Your account has been blocked. Reason: %s", reason);
        sendUserInboxMessage(acc, msg);
        snprintf(msg, sizeof(msg), "Admin blocked user %lld. Reason: %s", acc, reason);
        sendAdminInboxMessage(msg);

        char log[200];
        snprintf(log, sizeof(log), "Admin blocked user %lld. Reason: %s", acc, reason);
        auditLog(log);

        printf("User %lld blocked successfully.\n", acc);
    } else {
        strcpy(user.accountStatus, "Active");
        strcpy(user.cardStatus,    "Active");
        getCurrentDateTime(user.dateTime);
        updateUser(acc, user);

        sendUserInboxMessage(acc, "Your account has been unblocked by admin.");
        snprintf(msg, sizeof(msg), "Admin unblocked user %lld.", acc);
        sendAdminInboxMessage(msg);

        char log[200];
        snprintf(log, sizeof(log), "Admin unblocked user %lld.", acc);
        auditLog(log);

        printf("User %lld unblocked successfully.\n", acc);
    }
}

/* ================================================
   SEND MESSAGE TO USER (Admin)
   ================================================ */
void sendMessageToUserUI() {
    char input[20];
    long long acc;

    printf("Enter user account number: ");
    fgets(input, sizeof(input), stdin);
    if (sscanf(input, "%lld", &acc) != 1) { printf("Invalid!\n"); return; }

    User user;
    if (!readUser(acc, &user)) { printf("User not found!\n"); return; }

    char message[200];
    printf("Enter message: ");
    fgets(message, sizeof(message), stdin);
    message[strcspn(message, "\n")] = 0;

    sendUserInboxMessage(acc, message);

    char log[200];
    snprintf(log, sizeof(log), "Admin sent message to user %lld.", acc);
    auditLog(log);

    printf("Message sent to user %lld successfully.\n", acc);
}

/* ================================================
   DELETE USER UI
   ================================================ */
void deleteUserUI() {
    char input[20];
    long long acc;

    printf("Enter account number to delete: ");
    fgets(input, sizeof(input), stdin);
    if (sscanf(input, "%lld", &acc) != 1) { printf("Invalid!\n"); return; }

    if (deleteUser(acc)) {
        printf("User %lld deleted successfully.\n", acc);
        char log[200];
        snprintf(log, sizeof(log), "Admin deleted user %lld.", acc);
        auditLog(log);
    } else {
        printf("User not found!\n");
    }
}

/* ================================================
   ADD NEW USER (Admin UI wrapper)
   ================================================ */
void addNewUser() {
    User newUser;
    memset(&newUser, 0, sizeof(User));

    printf("Enter name: ");
    fgets(newUser.name, sizeof(newUser.name), stdin);
    newUser.name[strcspn(newUser.name, "\n")] = 0;

    char pinStr[10];
    printf("Enter MPIN (4-digit): ");
    fgets(pinStr, sizeof(pinStr), stdin);
    sscanf(pinStr, "%d", &newUser.mpin);

    if (newUser.mpin < 1000 || newUser.mpin > 9999) {
        printf("Invalid PIN! Must be 4 digits.\n");
        return;
    }

    printf("Enter ATM PIN (4-digit): ");
    fgets(pinStr, sizeof(pinStr), stdin);
    sscanf(pinStr, "%d", &newUser.atmPin);

    if (newUser.atmPin < 1000 || newUser.atmPin > 9999) {
        printf("Invalid ATM PIN! Must be 4 digits.\n");
        return;
    }

    printf("Enter CNIC: ");
    fgets(newUser.cnic, sizeof(newUser.cnic), stdin);
    newUser.cnic[strcspn(newUser.cnic, "\n")] = 0;

    addUser(newUser);

    char log[200];
    snprintf(log, sizeof(log), "Admin added new user: %s", newUser.name);
    auditLog(log);
}

/* ================================================
   CHANGE PIN
   ================================================ */
void changePin(long long accountNumber) {
    User user;
    if (!readUser(accountNumber, &user)) { printf("User not found!\n"); return; }

    char input[20];
    int oldPin, newPin, confirmPin;

    printf("Enter your current PIN: ");
    fgets(input, sizeof(input), stdin);
    if (sscanf(input, "%d", &oldPin) != 1 || oldPin != user.mpin) {
        printf("Incorrect current PIN!\n");
        return;
    }

    printf("Enter new PIN (4 digits): ");
    fgets(input, sizeof(input), stdin);
    if (sscanf(input, "%d", &newPin) != 1 || newPin < 1000 || newPin > 9999) {
        printf("Invalid PIN! Must be 4 digits.\n");
        return;
    }

    printf("Confirm new PIN: ");
    fgets(input, sizeof(input), stdin);
    if (sscanf(input, "%d", &confirmPin) != 1 || confirmPin != newPin) {
        printf("PINs do not match!\n");
        return;
    }

    user.mpin = newPin;
    getCurrentDateTime(user.dateTime);
    updateUser(accountNumber, user);
    sendUserInboxMessage(accountNumber, "Your PIN was changed successfully.");
    printf("PIN changed successfully!\n");
}

void changeMPIN(long long accountNumber) {
    changePin(accountNumber);
}

/* ================================================
   USER PANEL MENU
   ================================================ */
void userPanel(long long accountNo) {
    userPanelMenu(accountNo);
}

void userPanelMenu(long long accountNumber) {
    int choice;
    char input[10];

    while (1) {
        printf("\n--- User Panel ---\n");
        printf("1. View Inbox\n");
        printf("2. View Transactions\n");
        printf("3. View ATM Card Info\n");
        printf("4. View Balance\n");
        printf("5. Change PIN\n");
        printf("6. Logout\n");
        printf("Enter your choice: ");
        fgets(input, sizeof(input), stdin);
        if (sscanf(input, "%d", &choice) != 1) {
            printf("Invalid input!\n"); continue;
        }

        switch (choice) {
            case 1: viewUserInbox(accountNumber);        break;
            case 2: viewUserTransactions(accountNumber); break;
            case 3: viewAtmCardInfo(accountNumber);      break;
            case 4: viewBalance(accountNumber);          break;
            case 5: changePin(accountNumber);            break;
            case 6: printf("Logging out...\n");          return;
            default: printf("Invalid choice!\n");
        }
    }
}

/* ================================================
   ATM MENU
   ================================================ */
// FIX: atmUserMenu defined BEFORE atmMenu so no implicit declaration warning
void atmUserMenu(long long accountNumber) {
    int choice;
    char input[10];

    while (1) {
        printf("\n--- ATM Menu ---\n");
        printf("1. Deposit Money\n");
        printf("2. Withdraw Money\n");
        printf("3. Transfer Money\n");
        printf("4. View Balance\n");
        printf("5. Mini Statement\n");
        printf("6. Logout\n");
        printf("Enter your choice: ");
        fgets(input, sizeof(input), stdin);
        if (sscanf(input, "%d", &choice) != 1) {
            printf("Invalid input!\n"); continue;
        }

        switch (choice) {
            case 1: depositAmount(accountNumber);  break;
            case 2: withdrawAmount(accountNumber); break;
            case 3: transferAmount(accountNumber); break;
            case 4: viewBalance(accountNumber);    break;
            case 5: miniStatement(accountNumber);  break;
            case 6: printf("Logging out...\n");    return;
            default: printf("Invalid choice!\n");
        }
    }
}

/* ================================================
   ATM MENU WRAPPER
   ================================================ */
// FIX: atmMenu defined AFTER atmUserMenu to avoid implicit declaration
void atmMenu(long long atmCardNo) {
    atmUserMenu(atmCardNo);
}

/* ================================================
   USER PANEL LOGIN
   ================================================ */
long long userPanelLogin() {
    char input[20];
    long long acc;
    int pin;

    printf("Enter Account Number: ");
    fgets(input, sizeof(input), stdin);
    if (sscanf(input, "%lld", &acc) != 1) return 0;

    printf("Enter PIN: ");
    fgets(input, sizeof(input), stdin);
    if (sscanf(input, "%d", &pin) != 1) return 0;

    User user;
    if (readUser(acc, &user) && user.mpin == pin) {
        if (strcmp(user.accountStatus, "Blocked") == 0) {
            printf("Account is blocked!\n");
            return 0;
        }
        printf("Login Successful! Welcome, %s\n", user.name);
        return acc;
    }

    printf("Invalid credentials!\n");
    return 0;
}

/* ================================================
   SEND USER NOTIFICATION
   ================================================ */
void sendUserNotification(long long accountNumber, const char *message) {
    sendUserInboxMessage(accountNumber, message);
}

/* ================================================
   INBOX FUNCTIONS (user.h prototypes)
   ================================================ */
void viewInbox(long long accNo) {
    viewUserInbox(accNo);
}

/* ================================================
   GENERATE ACCOUNT / ATM CARD NUMBER
   ================================================ */
long long generateAccountNo() {
    FILE *fp = fopen(USER_FILE, "rb");
    long long lastID = 1000;
    if (fp) {
        User temp;
        while (fread(&temp, sizeof(User), 1, fp))
            if (temp.accountNo > lastID)
                lastID = temp.accountNo;
        fclose(fp);
    }
    return lastID + 1;
}

long long generateAtmCardNo() {
    FILE *fp = fopen(USER_FILE, "rb");
    long long lastID = 50000;
    if (fp) {
        User temp;
        while (fread(&temp, sizeof(User), 1, fp))
            if (temp.atmCardNo > lastID)
                lastID = temp.atmCardNo;
        fclose(fp);
    }
    return lastID + 1;
}

int countUsers() {
    FILE *fp = fopen(USER_FILE, "rb");
    if (!fp) return 0;
    int count = 0;
    User temp;
    while (fread(&temp, sizeof(User), 1, fp)) count++;
    fclose(fp);
    return count;
}

void getDateTime(char *buffer) {
    getCurrentDateTime(buffer);
}

/* ================================================
   ADMIN PANEL
   ================================================ */
void adminPanel() {
    int choice;
    char input[10];

    while (1) {
        printf("\n========== Admin Panel ==========\n");
        printf("1.  View All Users\n");
        printf("2.  Add New User\n");
        printf("3.  Delete User\n");
        printf("4.  View User Transactions\n");
        printf("5.  View Admin Inbox\n");
        printf("6.  Send Message to User\n");
        printf("7.  Block / Unblock User\n");
        printf("8.  Search User\n");
        printf("9.  Edit User Info\n");
        printf("10. View Blocked Users\n");
        printf("11. View Audit Log\n");
        printf("12. Logout\n");
        printf("=================================\n");
        printf("Enter your choice: ");

        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;
        if (sscanf(input, "%d", &choice) != 1) {
            printf("Invalid input!\n"); continue;
        }

        switch (choice) {
            case 1:  viewAllUsers();           break;
            case 2:  addNewUser();             break;
            case 3:  deleteUserUI();           break;
            case 4:  viewUserTransactionsUI(); break;
            case 5:  viewAdminInbox();         break;
            case 6:  sendMessageToUserUI();    break;
            case 7:  toggleUserBlockStatus();  break;
            case 8:  searchUserUI();           break;
            case 9:  editUserInfoUI();         break;
            case 10: viewBlockedUsers();       break;
            case 11: viewAuditLog();           break;
            case 12: printf("Logging out...\n"); return;
            default: printf("Invalid choice!\n");
        }
    }
}
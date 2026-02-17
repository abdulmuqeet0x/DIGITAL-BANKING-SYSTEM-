#include "atm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "user.h"

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
/* getch() is defined in user.c — declare here so atm.c can use it */
int getch(void);
#endif

/* inputMaskedPin() and inputATMPIN() are defined in user.c — declare here */
int inputMaskedPin(void);
int inputATMPIN(void);

/* logTransaction is defined in user.c */
void logTransaction(long long accNo, const char *type, double amount, long long otherAcc);
/* sendAdminNotification is defined in user.c */
void sendAdminNotification(const char *msg);
void sendUserNotification(long long accNo, const char *msg);

#define TEMP_FILE "temp_userpanel.txt"

/* ---------- ATM WITHDRAW ---------- */
void atmWithdraw(long long atmCardNo) {
    FILE *ufp  = fopen(USER_FILE, "r");
    FILE *temp = fopen(TEMP_FILE, "w");
    if (!ufp || !temp) {
        if (ufp)  fclose(ufp);
        if (temp) fclose(temp);
        printf(RED "File error!\n" RESET);
        return;
    }

    char line[300];
    double withdrawAmount;
    int found       = 0;
    int insufficient = 0;

    printf("Enter amount to withdraw: ");
    scanf("%lf", &withdrawAmount);
    getchar();

    if (withdrawAmount <= 0) {
        printf(RED "Invalid amount! Must be greater than zero.\n" RESET);
        fclose(ufp); fclose(temp);
        remove(TEMP_FILE);
        return;
    }

    int inTarget = 0;
    while (fgets(line, sizeof(line), ufp)) {
        if (strstr(line, "AccountNo   :")) {
            long long acc;
            sscanf(line, "AccountNo   : %lld", &acc);
            inTarget = (acc == atmCardNo);
            if (inTarget) found = 1;
        }

        if (inTarget && strstr(line, "Balance     :")) {
            double bal;
            sscanf(line, "Balance     : %lf", &bal);
            if (withdrawAmount > bal) {
                printf(RED "Insufficient Balance! Available: %.2lf PKR\n" RESET, bal);
                insufficient = 1;
                fprintf(temp, "Balance     : %.2lf\n", bal);
            } else {
                bal -= withdrawAmount;
                fprintf(temp, "Balance     : %.2lf\n", bal);
                printf(GREEN "Withdraw Successful! New Balance: %.2lf PKR\n" RESET, bal);
                logTransaction(atmCardNo, "Withdraw", withdrawAmount, 0);
                char umsg[150];
                snprintf(umsg, sizeof(umsg), "Withdraw of %.2lf PKR successful. New Balance: %.2lf PKR", withdrawAmount, bal);
                sendUserNotification(atmCardNo, umsg);
            }
            inTarget = 0;
            continue;
        }

        fputs(line, temp);
    }

    fclose(ufp);
    fclose(temp);

    if (insufficient) {
        remove(TEMP_FILE); /* discard temp, keep original */
    } else {
        remove(USER_FILE);
        rename(TEMP_FILE, USER_FILE);
    }

    if (!found) printf(RED "Account not found!\n" RESET);
}

/* ---------- ATM DEPOSIT ---------- */
void atmDeposit(long long atmCardNo) {
    FILE *ufp  = fopen(USER_FILE, "r");
    FILE *temp = fopen(TEMP_FILE, "w");
    if (!ufp || !temp) {
        if (ufp)  fclose(ufp);
        if (temp) fclose(temp);
        printf(RED "File error!\n" RESET);
        return;
    }

    char line[300];
    double depositAmount;
    int found = 0;

    printf("Enter amount to deposit: ");
    scanf("%lf", &depositAmount);
    getchar();

    if (depositAmount <= 0) {
        printf(RED "Invalid amount! Must be greater than zero.\n" RESET);
        fclose(ufp); fclose(temp);
        remove(TEMP_FILE);
        return;
    }

    int inTarget = 0;
    while (fgets(line, sizeof(line), ufp)) {
        if (strstr(line, "AccountNo   :")) {
            long long acc;
            sscanf(line, "AccountNo   : %lld", &acc);
            inTarget = (acc == atmCardNo);
            if (inTarget) found = 1;
        }

        if (inTarget && strstr(line, "Balance     :")) {
            double bal;
            sscanf(line, "Balance     : %lf", &bal);
            bal += depositAmount;
            fprintf(temp, "Balance     : %.2lf\n", bal);
            printf(GREEN "Deposit Successful! New Balance: %.2lf PKR\n" RESET, bal);
            logTransaction(atmCardNo, "Deposit", depositAmount, 0);
            char umsg[150];
            snprintf(umsg, sizeof(umsg), "Deposit of %.2lf PKR successful. New Balance: %.2lf PKR", depositAmount, bal);
            sendUserNotification(atmCardNo, umsg);
            inTarget = 0;
            continue;
        }

        fputs(line, temp);
    }

    fclose(ufp);
    fclose(temp);
    remove(USER_FILE);
    rename(TEMP_FILE, USER_FILE);

    if (!found) printf(RED "Account not found!\n" RESET);
}

/* ---------- ATM TRANSFER ---------- */
void atmTransfer(long long fromAtmCardNo) {
    long long toAccount;
    double amount;

    printf("Enter recipient Account Number: ");
    scanf("%lld", &toAccount);
    getchar();

    if (toAccount == fromAtmCardNo) {
        printf(RED "Cannot transfer to your own account!\n" RESET);
        return;
    }

    printf("Enter amount to transfer: ");
    scanf("%lf", &amount);
    getchar();

    if (amount <= 0) {
        printf(RED "Invalid amount! Must be greater than zero.\n" RESET);
        return;
    }

    /* Dynamic buffer — avoids 1.5 MB stack array */
    int   capacity   = 1000;
    char (*buffer)[300] = malloc(capacity * sizeof(*buffer));
    if (!buffer) { printf(RED "Memory allocation failed!\n" RESET); return; }
    int totalLines = 0;

    FILE *ufp = fopen(USER_FILE, "r");
    if (!ufp) {
        printf(RED "File error!\n" RESET);
        free(buffer);
        return;
    }

    char line[300];
    int  inFrom = 0, inTo = 0;
    int  fromFound = 0, toFound = 0;

    while (fgets(line, sizeof(line), ufp)) {
        if (totalLines >= capacity) {
            capacity *= 2;
            char (*tmp)[300] = realloc(buffer, capacity * sizeof(*buffer));
            if (!tmp) {
                printf(RED "Memory error!\n" RESET);
                fclose(ufp); free(buffer); return;
            }
            buffer = tmp;
        }

        strcpy(buffer[totalLines], line);

        if (strstr(line, "account :")) {
            long long acc;
            sscanf(line, "account : %lld", &acc);
            inFrom = (acc == fromAtmCardNo);
            inTo   = (acc == toAccount);
            if (inFrom) fromFound = 1;
            if (inTo)   toFound   = 1;
        }

        if (inFrom && strstr(line, "balance :")) {
            double fromBal;
            sscanf(line, "balance : %lf", &fromBal);
            if (amount > fromBal) {
                printf(RED "Insufficient balance! Available: %.2lf PKR\n" RESET, fromBal);
                fclose(ufp); free(buffer); return;
            }
            fromBal -= amount;
            sprintf(buffer[totalLines], "balance : %.2lf\n", fromBal);
            inFrom = 0;
        }

        if (inTo && strstr(line, "balance :")) {
            double toBal;
            sscanf(line, "balance : %lf", &toBal);
            toBal += amount;
            sprintf(buffer[totalLines], "balance : %.2lf\n", toBal);
            inTo = 0;
        }

        totalLines++;
    }
    fclose(ufp);

    if (!fromFound) { printf(RED "Sender account not found!\n" RESET);    free(buffer); return; }
    if (!toFound)   { printf(RED "Recipient account not found!\n" RESET); free(buffer); return; }

    FILE *temp = fopen(TEMP_FILE, "w");
    if (!temp) { printf(RED "File error!\n" RESET); free(buffer); return; }

    for (int i = 0; i < totalLines; i++)
        fputs(buffer[i], temp);

    fclose(temp);
    free(buffer);
    remove(USER_FILE);
    rename(TEMP_FILE, USER_FILE);

    logTransaction(fromAtmCardNo, "Transfer Sent",     amount, toAccount);
    logTransaction(toAccount,      "Transfer Received", amount, fromAtmCardNo);

    char smsg[150], rmsg[150], adminMsg[200];
    snprintf(smsg, sizeof(smsg),
        "Transfer of %.2lf PKR sent to account %lld.", amount, toAccount);
    snprintf(rmsg, sizeof(rmsg),
        "Transfer of %.2lf PKR received from account %lld.", amount, fromAtmCardNo);
    snprintf(adminMsg, sizeof(adminMsg),
        "TRANSFER | From: %lld | To: %lld | Amount: %.2lf PKR",
        fromAtmCardNo, toAccount, amount);

    sendUserNotification(fromAtmCardNo, smsg);
    sendUserNotification(toAccount,     rmsg);
    sendAdminNotification(adminMsg);

    printf(GREEN "Transfer Successful! %.2lf PKR sent to account %lld\n" RESET,
           amount, toAccount);
}

/* ---------- ATM BALANCE INQUIRY ---------- */
void atmBalanceInquiry(long long atmCardNo) {
    FILE *fp = fopen(USER_FILE, "r");
    if (!fp) { printf(RED "User panel file not found!\n" RESET); return; }

    char line[300];
    int  found = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "AccountNo   :")) {
            long long acc;
            sscanf(line, "AccountNo   : %lld", &acc);
            found = (acc == atmCardNo);
        }
        if (found && strstr(line, "Balance     :")) {
            double balance;
            sscanf(line, "Balance     : %lf", &balance);
            printf(GREEN "Your Balance: %.2lf PKR\n" RESET, balance);
            break;
        }
    }
    fclose(fp);

    if (!found) printf(RED "Account not found!\n" RESET);
}

/* ---------- ATM MINI STATEMENT ---------- */
void atmMiniStatement(long long atmCardNo) {
    FILE *fp = fopen(TRANSACTION_FILE, "r");
    if (!fp) { printf(YELLOW "No transactions found.\n" RESET); return; }

    /* collect all matching lines then show last 5 */
    char lines[100][300];
    int  count = 0;
    char line[300];

    int inRec2 = 0;
    char rec2[512];
    long long fa2 = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "----------------------------") && !inRec2) {
            inRec2 = 1; rec2[0] = '\0'; fa2 = 0;
            strncat(rec2, line, 511); continue;
        }
        if (inRec2) {
            strncat(rec2, line, 511 - strlen(rec2));
            if (strstr(line, "AccountNo        :")) sscanf(line, "AccountNo        : %lld", &fa2);
            if (strstr(line, "----------------------------") && strlen(rec2) > 30) {
                if (fa2 == atmCardNo) { strncpy(lines[count % 100], rec2, 299); count++; }
                inRec2 = 0;
            }
        }
    }
    fclose(fp);

    if (count == 0) { printf(YELLOW "No transactions found for this card.\n" RESET); return; }

    int start = (count > 5) ? count - 5 : 0;
    printf(CYAN "\n====== MINI STATEMENT (Last 5) ======\n" RESET);
    for (int i = start; i < count && i < 100; i++)
        printf("%s", lines[i % 100]);
}

/* ---------- ATM LOGIN ---------- */
long long atmLogin() {
    char cardInput[30];
    long long atmNo;
    int attempts = 0;

    printf("Enter ATM Card Number (or 0 to cancel): ");
    if (fgets(cardInput, sizeof(cardInput), stdin) == NULL) return 0;
    cardInput[strcspn(cardInput, "\n")] = 0;

    if (strcmp(cardInput, "0") == 0) {
        printf(YELLOW "ATM login cancelled.\n" RESET);
        return 0;
    }
    if (sscanf(cardInput, "%lld", &atmNo) != 1) {
        printf(RED "Invalid card number!\n" RESET);
        return 0;
    }

    printf(CYAN "Tip: Enter 0 as PIN at any time to cancel.\n" RESET);

    while (attempts < 3) {
        printf("Enter ATM PIN (attempts left: %d, or 0 to exit): ", 3 - attempts);
        int atmPin = inputMaskedPin();

        /* 0000 as pin = exit signal */
        if (atmPin == 0) {
            printf(YELLOW "ATM login cancelled.\n" RESET);
            return 0;
        }

        FILE *fp = fopen(ATM_FILE, "r");
        if (!fp) { printf(RED "ATM card file not found!\n" RESET); return 0; }

        char line[300];
        long long fileATM = 0;
        int filePIN = 0, success = 0;

        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, "AtmCard No  :")) sscanf(line, "AtmCard No  : %lld", &fileATM);
            if (strstr(line, "ATM PIN     :")) sscanf(line, "ATM PIN     : %d",   &filePIN);
            if (fileATM == atmNo && filePIN == atmPin) { success = 1; break; }
        }
        fclose(fp);

        if (success) {
            printf(GREEN "ATM Login successful!\n" RESET);
            return atmNo;
        }

        attempts++;
        if (attempts < 3)
            printf(RED "Wrong PIN! Attempts left: %d\n" RESET, 3 - attempts);
    }

    printf(RED "Card blocked due to too many failed attempts!\n" RESET);
    return 0;
}       
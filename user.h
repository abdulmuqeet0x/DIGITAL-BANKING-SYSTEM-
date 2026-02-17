#ifndef USER_H
#define USER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ---------- FILE NAMES ---------- */
#ifndef USER_FILE
#define USER_FILE        "userpanel.txt"
#endif
#ifndef ATM_FILE
#define ATM_FILE         "atmcard.txt"
#endif
#ifndef TRANSACTION_FILE
#define TRANSACTION_FILE "transactions.txt"
#endif
#ifndef USER_INBOX_FILE
#define USER_INBOX_FILE  "inbox_user.txt"
#endif
#ifndef ADMIN_INBOX_FILE
#define ADMIN_INBOX_FILE "inbox_admin.txt"   // FIX: matched to actual filename
#endif
#ifndef AUDIT_LOG_FILE
#define AUDIT_LOG_FILE   "auditlog.txt"
#endif

/* ---------- COLOR CODES ---------- */
#ifndef RESET
#define RESET   "\033[0m"
#endif
#ifndef RED
#define RED     "\033[1;31m"
#endif
#ifndef GREEN
#define GREEN   "\033[1;32m"
#endif
#ifndef YELLOW
#define YELLOW  "\033[1;33m"
#endif
#ifndef CYAN
#define CYAN    "\033[1;36m"
#endif

/* ---------- USER STRUCT ---------- */
typedef struct {
    long long accountNo;
    char      name[50];
    char      cnic[16];        // FIX: 13 digits + 2 dashes + null = 16 (was oversized at 20)
    int       mpin;
    int       atmPin;
    long long atmCardNo;
    double    balance;
    char      accountStatus[10]; // ACTIVE/INACTIVE
    char      cardStatus[10];    // ACTIVE/INACTIVE
    char      dateTime[40];      // account creation date
} User;

/* ---------- FUNCTION PROTOTYPES ---------- */

/* user.c */
int       countUsers();
long long generateAccountNo();
long long generateAtmCardNo();
void      getDateTime(char *buffer);

int  inputMaskedPin();
int  inputMPIN();
int  inputATMPIN();

void      addNewUser();
void      deleteUserUI();
void      viewAllUsers();
long long userPanelLogin();
long long atmLogin();              // FIX: was int, must be long long to hold account/card number
void      userPanelMenu(long long accNo);

void viewAccountDetails(long long accNo);
void viewBalance(long long accNo);
void viewATMCard(long long accNo);
void changeMPIN(long long accNo);

/* Notification */
void sendUserNotification(long long accountNo, const char *message);

/* atm.c */
void withdrawCash(long long atmCardNo);
void depositCash(long long atmCardNo);
void transferMoney(long long atmCardNo);   // FIX: consistent parameter name with atm.h
void atmBalanceInquiry(long long atmCardNo);

void viewTransactionHistory(long long accNo);
void viewInbox(long long accNo);
void miniStatement(long long accNo);

#endif /* USER_H */
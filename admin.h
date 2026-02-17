#ifndef ADMIN_H
#define ADMIN_H

#include "user.h"   // for User struct & USER_FILE

/* -------------------- FILES -------------------- */
#ifndef ADMIN_INBOX_FILE
#define ADMIN_INBOX_FILE "inbox_admin.txt"   // FIX: matched to actual filename
#endif

#ifndef ADMIN_FILE
#define ADMIN_FILE "adminpanel.txt"
#endif

#ifndef AUDIT_LOG_FILE
#define AUDIT_LOG_FILE "auditlog.txt"
#endif

/* -------------------- COLOR CODES -------------------- */
#define RESET   "\033[0m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define CYAN    "\033[1;36m"

/* -------------------- ADMIN STRUCT -------------------- */
typedef struct {
    char name[50];
    int  adminID;       // auto generated
    int  pin;           // 4-digit PIN (stored as plain int – fine for a project)
    char cnic[16];      // FIX: 13 digits + 2 dashes + null = 16  (e.g. "XXXXX-XXXXXXX-X")
                        //      If you store digits-only (no dashes) change back to char cnic[14]
    char dateTime[30];  // creation datetime
} Admin;

/* -------------------- ADMIN AUTH FUNCTIONS -------------------- */
void adminEntry();                   // initial admin entry (create/login)
int  adminLogin();                   // login flow
void createAdmin();                  // create new admin account
int  validateAdminPin(int savedPin); // check pin
int  generateAdminID();              // generate unique admin ID
void printAdminCard(Admin admin);    // print admin info card

/* -------------------- ADMIN PANEL FUNCTIONS -------------------- */
void adminPanel();                   // main admin panel
void viewAllUsers();                 // list all users
void addNewUser();                   // add new user (calls user.c)
void deleteUserUI();                 // delete user (calls user.c)
void viewUserTransactionsUI();       // view transactions of a user
void sendMessageToUserUI();          // send message to a specific user
void toggleUserBlockStatus();        // block/unblock user account
void searchUserUI();                 // search user by name/account
void editUserInfoUI();               // edit user details
void viewBlockedUsers();             // list blocked users
void viewAuditLog();                 // view audit log file

/* -------------------- ADMIN INBOX FUNCTIONS -------------------- */
void sendAdminInboxMessage(const char *msg); // send message to admin inbox
void viewAdminInbox();                       // read admin inbox messages

/* -------------------- UTILITY FUNCTIONS -------------------- */
void getCurrentDateTime(char *buffer);       // get current date & time string

#endif /* ADMIN_H */
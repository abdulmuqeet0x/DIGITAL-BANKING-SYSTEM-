#include "admin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
  #include <conio.h>
#else
  #include <termios.h>
  #include <unistd.h>
  int getch(void);   /* defined in user.c */
#endif

/* defined in user.c */
int inputMaskedPin(void);
int inputATMPIN(void);
int isCnicDuplicate(const char *cnic);

/*
   adminpanel.txt format:
   ==============================
   AdminName   : Abdul Muqeet
   AdminID     : 1030001
   PIN         : 1234
   CNIC        : 12345-6789012-3
   Date & Time : Tue Feb 17 16:41:16 2026
   ==============================

*/

/* ================================================================
   UTILITY
================================================================ */
void getCurrentDateTime(char *buffer) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buffer, 30, "%a %b %d %H:%M:%S %Y", tm_info);
}

void logAudit(const char *message) {
    FILE *fp = fopen(AUDIT_LOG_FILE, "a");
    if (!fp) return;
    char dt[30];
    getCurrentDateTime(dt);
    fprintf(fp, "[%s] %s\n", dt, message);
    fclose(fp);
}

/* ================================================================
   READ ONE ADMIN RECORD FROM FILE
   Returns 1 if found, 0 if EOF
================================================================ */
static int readNextAdmin(FILE *fp, Admin *a) {
    char line[200];
    int  got = 0;

    memset(a, 0, sizeof(Admin));

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0;

        if (strstr(line, "AdminName   :")) {
            sscanf(line, "AdminName   : %49[^\n]", a->name);
            /* name may have spaces — use the part after ": " */
            char *p = strstr(line, ": ");
            if (p) strncpy(a->name, p + 2, 49);
            got++;
        }
        else if (strstr(line, "AdminID     :")) { sscanf(line, "AdminID     : %d",  &a->adminID);  got++; }
        else if (strstr(line, "PIN         :")) { sscanf(line, "PIN         : %d",  &a->pin);       got++; }
        else if (strstr(line, "CNIC        :")) {
            char *p = strstr(line, ": ");
            if (p) strncpy(a->cnic, p + 2, 15);
            got++;
        }
        else if (strstr(line, "Date & Time :")) {
            char *p = strstr(line, ": ");
            if (p) strncpy(a->dateTime, p + 2, 29);
            got++;
        }
        else if (strstr(line, "==============================") && got > 0) {
            return 1;   /* end of record */
        }
    }
    return (got > 0) ? 1 : 0;
}

/* ================================================================
   GENERATE ADMIN ID  (reads text file, finds max ID)
================================================================ */
int generateAdminID() {
    FILE *fp = fopen(ADMIN_FILE, "r");
    int lastID = 1030000;
    if (fp) {
        Admin tmp;
        while (readNextAdmin(fp, &tmp))
            if (tmp.adminID > lastID) lastID = tmp.adminID;
        fclose(fp);
    }
    return lastID + 1;
}

/* ================================================================
   PRINT ADMIN CARD  (screen only)
================================================================ */
void printAdminCard(Admin admin) {
    printf(CYAN "\n==============================\n"   RESET);
    printf(CYAN "         ADMIN CARD\n"               RESET);
    printf(CYAN "==============================\n"    RESET);
    printf("AdminName   : %s\n", admin.name);
    printf("AdminID     : %d\n", admin.adminID);
    printf("PIN         : ****\n");
    printf("CNIC        : %s\n", admin.cnic);
    printf("Date & Time : %s\n", admin.dateTime);
    printf(CYAN "==============================\n\n"  RESET);
}

/* ================================================================
   CREATE ADMIN  (writes text record)
================================================================ */
void createAdmin() {
    Admin admin;

    printf("Enter Admin Name             : ");
    fgets(admin.name, sizeof(admin.name), stdin);
    admin.name[strcspn(admin.name, "\n")] = 0;

    printf("Enter CNIC (XXXXX-XXXXXXX-X) : ");
    fgets(admin.cnic, sizeof(admin.cnic), stdin);
    admin.cnic[strcspn(admin.cnic, "\n")] = 0;

    int cnicLen = (int)strlen(admin.cnic);
    if (cnicLen != 15 && cnicLen != 13) {
        printf(RED "Invalid CNIC! Use XXXXX-XXXXXXX-X format.\n" RESET);
        return;
    }

    /* DUPLICATE CNIC CHECK */
    if (isCnicDuplicate(admin.cnic)) {
        printf(RED "Error: CNIC %s is already registered! Cannot create another account.\n" RESET, admin.cnic);
        return;
    }

    printf("Set 4-digit PIN              : ");
    admin.pin     = inputMaskedPin();
    admin.adminID = generateAdminID();
    getCurrentDateTime(admin.dateTime);

    /* ---- write text record ---- */
    FILE *fp = fopen(ADMIN_FILE, "a");   /* text append */
    if (!fp) { printf(RED "Cannot open admin file!\n" RESET); return; }

    fprintf(fp,
        "==============================\n"
        "AdminName   : %s\n"
        "AdminID     : %d\n"
        "PIN         : %d\n"
        "CNIC        : %s\n"
        "Date & Time : %s\n"
        "==============================\n\n",
        admin.name, admin.adminID, admin.pin,
        admin.cnic, admin.dateTime);
    fclose(fp);

    printf(GREEN "\nAdmin created successfully!\n" RESET);
    printAdminCard(admin);

    char log[200];
    snprintf(log, sizeof(log), "Admin '%s' created with ID %d.", admin.name, admin.adminID);
    logAudit(log);
}

/* ================================================================
   VALIDATE PIN
================================================================ */
int validateAdminPin(int savedPin) {
    int attempts = 0, entered;
    while (attempts < 3) {
        printf("Enter 4-digit PIN: ");
        entered = inputMaskedPin();
        if (entered == savedPin) return 1;
        attempts++;
        printf(RED "Incorrect PIN! Attempts left: %d\n" RESET, 3 - attempts);
    }
    printf(RED "Access denied!\n" RESET);
    return 0;
}

/* ================================================================
   CHECK CREDENTIALS  (reads text file)
================================================================ */
int checkAdminCredentials(char *secretID, int pin) {
    FILE *fp = fopen(ADMIN_FILE, "r");
    if (!fp) return 0;

    Admin a;
    char  idStr[20];
    while (readNextAdmin(fp, &a)) {
        sprintf(idStr, "%d", a.adminID);
        if (strcmp(idStr, secretID) == 0 && a.pin == pin) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

/* ================================================================
   GET ADMIN BY ID  (reads text file)
================================================================ */
int getAdminByID(char *secretID, Admin *out) {
    FILE *fp = fopen(ADMIN_FILE, "r");
    if (!fp) return 0;

    Admin a;
    char  idStr[20];
    while (readNextAdmin(fp, &a)) {
        sprintf(idStr, "%d", a.adminID);
        if (strcmp(idStr, secretID) == 0) {
            fclose(fp);
            *out = a;
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

/* ================================================================
   ADMIN LOGIN
================================================================ */
int adminLogin() {
    FILE *fp = fopen(ADMIN_FILE, "r");
    if (!fp) {
        printf(YELLOW "No admin account found. Please create one first.\n" RESET);
        return 0;
    }
    fclose(fp);

    char secretID[20];
    int  pin, attempts = 0;

    printf(CYAN "Tip: Enter 0 as Secret ID to cancel.\n" RESET);

    while (attempts < 3) {
        printf("\nEnter Admin ID (or 0 to exit): ");
        fgets(secretID, sizeof(secretID), stdin);
        secretID[strcspn(secretID, "\n")] = 0;

        if (strcmp(secretID, "0") == 0) {
            printf(YELLOW "Login cancelled.\n" RESET);
            return 0;
        }

        printf("Enter PIN (attempts left: %d): ", 3 - attempts);
        pin = inputMaskedPin();

        if (checkAdminCredentials(secretID, pin)) {
            Admin admin;
            if (getAdminByID(secretID, &admin)) {
                printf(GREEN "\nLogin successful!\n" RESET);
                printAdminCard(admin);
                char log[200];
                snprintf(log, sizeof(log), "Admin '%s' (ID %d) logged in.",
                         admin.name, admin.adminID);
                logAudit(log);
            }
            return 1;
        }

        attempts++;
        if (attempts < 3)
            printf(RED "Invalid ID or PIN! Attempts left: %d\n" RESET, 3 - attempts);
    }
    printf(RED "Access denied — too many failed attempts.\n" RESET);
    return 0;
}

/* ================================================================
   ADMIN ENTRY  (not used in main flow but kept for compatibility)
================================================================ */
void adminEntry() {
    char choice[10];
    printf("Do you have an admin account? (yes/no): ");
    fgets(choice, sizeof(choice), stdin);
    choice[strcspn(choice, "\n")] = 0;

#ifdef _WIN32
    if      (_stricmp(choice, "yes") == 0) { if (adminLogin()) adminPanel(); }
    else if (_stricmp(choice, "no")  == 0) {
#else
    if      (strcasecmp(choice, "yes") == 0) { if (adminLogin()) adminPanel(); }
    else if (strcasecmp(choice, "no")  == 0) {
#endif
        createAdmin();
        printf(GREEN "Account created! Please login now.\n" RESET);
        if (adminLogin()) adminPanel();
    } else {
        printf(RED "Invalid choice.\n" RESET);
    }
}

/* ================================================================
   ADMIN PANEL
================================================================ */
void adminPanel() {
    int  choice;
    char input[10];

    while (1) {
        printf(CYAN "\n========== Admin Panel ==========\n" RESET);
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
        if (sscanf(input, "%d", &choice) != 1) {
            printf(RED "Invalid input.\n" RESET);
            continue;
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
            case 12: printf(YELLOW "Logging out...\n" RESET); return;
            default: printf(RED "Invalid choice.\n" RESET);
        }
    }
}
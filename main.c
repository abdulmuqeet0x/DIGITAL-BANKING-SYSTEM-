#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "admin.h"
#include "user.h"
#include "atm.h"

/* ================================================================
   ADMIN SUBMENU  — shown when user picks "Admin Panel"
   Gives choice: Create new admin  OR  Login with existing account
================================================================ */
void adminSubmenu() {
    char input[10];
    int  choice;

    while (1) {
        printf("\n=================================\n");
        printf("        ADMIN PANEL\n");
        printf("=================================\n");
        printf("1. Create Admin Account\n");
        printf("2. Admin Login\n");
        printf("3. Back to Main Menu\n");
        printf("Enter your choice: ");

        if (fgets(input, sizeof(input), stdin) == NULL) continue;
        input[strcspn(input, "\n")] = '\0';

        if (sscanf(input, "%d", &choice) != 1) {
            printf(RED "Invalid input! Enter 1, 2 or 3.\n" RESET);
            continue;
        }

        switch (choice) {
            case 1:
                /* Create account, then MUST login — no auto panel access */
                createAdmin();
                printf(GREEN "Account created! Please login now.\n" RESET);
                if (adminLogin()) adminPanel();
                return;

            case 2:
                /* Login — adminLogin() returns 1 on success, 0 on fail/exit */
                if (adminLogin())
                    adminPanel();
                return;

            case 3:
                return; /* back to main menu */

            default:
                printf(RED "Invalid choice! Enter 1, 2 or 3.\n" RESET);
        }
    }
}

/* ================================================================
   MAIN MENU
================================================================ */
void mainMenu() {
    int  choice;
    char input[20];

    while (1) {
        printf("\n=================================\n");
        printf("     DIGITAL BANKING SYSTEM\n");
        printf("=================================\n");
        printf("1. Admin Panel\n");
        printf("2. User Panel Login\n");
        printf("3. ATM Services\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Input error! Try again.\n");
            continue;
        }
        input[strcspn(input, "\n")] = '\0';

        if (sscanf(input, "%d", &choice) != 1) {
            printf(RED "Invalid input! Please enter a number between 1-4.\n" RESET);
            continue;
        }

        switch (choice) {

            /* -------- ADMIN -------- */
            case 1:
                adminSubmenu();
                break;

            /* -------- USER -------- */
            case 2: {
                long long userAccNo = userPanelLogin();
                if (userAccNo != 0)
                    userPanelMenu(userAccNo);
                break;
            }

            /* -------- ATM -------- */
            case 3: {
                /* atmLogin() handles card number + PIN internally.
                   It returns the card number on success, 0 on failure/exit. */
                long long cardNo = atmLogin();
                if (cardNo != 0)
                    atmMenu(cardNo);
                break;
            }

            /* -------- EXIT -------- */
            case 4:
                printf("\nThank you for using Digital Banking System!\n");
                exit(0);

            default:
                printf(RED "Invalid choice! Please select between 1-4.\n" RESET);
        }
    }
}

/* ================================================================
   ENTRY POINT
================================================================ */
int main() {
    srand((unsigned int)time(NULL)); /* seed random for account/card number generation */

    printf("\n=================================\n");
    printf("  WELCOME TO DIGITAL BANKING SYSTEM\n");
    printf("===================================\n");

    mainMenu();
    return 0;
}
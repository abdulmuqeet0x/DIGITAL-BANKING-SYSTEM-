#ifndef ATM_H
#define ATM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "user.h"   // To access User struct and user-related functions

/* ---------------------------- COLOR CODES ---------------------------- */
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

/* ---------------------------- ATM FUNCTION PROTOTYPES ---------------------------- */

// ---------------------------- User Panel Functions ----------------------------
void      userPanel(long long accountNo);              // Main user panel menu
void      checkBalance(User user);                     // View user balance
void      depositMoney(User *user);                    // Deposit cash
void      withdrawMoney(User *user);                   // Withdraw cash
void      transferMoney(long long accountNo);          // Transfer money to another account
int       verifyUserPin(long long accountNo, int pin); // Verify MPIN/ATM PIN

// ---------------------------- ATM Specific Functions ----------------------------
long long atmLogin();                                  // FIX: was int, must be long long to hold account number
void      atmMenu(long long atmCardNo);                // ATM menu
void      atmBalanceInquiry(long long atmCardNo);      // Check balance via ATM
void      atmWithdraw(long long atmCardNo);            // Withdraw via ATM
void      atmDeposit(long long atmCardNo);             // Deposit via ATM
void      atmTransfer(long long atmCardNo);            // Transfer via ATM
void      atmMiniStatement(long long atmCardNo);       // Print mini-statement

#endif /* ATM_H */
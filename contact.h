#ifndef CONTACT_H
#define CONTACT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

/**
 * Singly Linked List Node for Contact Management System
 * Core Data Structure: Singly Linked List in C
 */
struct Contact {
    int id;                 /* Unique contact identifier */
    char name[50];          /* Contact Full Name */
    char phone[20];         /* Contact Phone Number (string preserves leading zeros) */
    char email[60];         /* Contact Email Address */
    struct Contact *next;   /* Pointer to the next node in the singly linked list */
};

/* Type definition for convenience */
typedef struct Contact Contact;

/* --- Core Singly Linked List Prototypes --- */

/**
 * Creates and allocates a new Contact node using malloc().
 * Returns pointer to new node, or NULL on memory allocation failure.
 */
Contact* createNode(int id, const char *name, const char *phone, const char *email);

/**
 * Adds a contact to the end of the linked list.
 * Validates non-empty fields and duplicate phone numbers.
 * Assigns next available ID if passed id <= 0.
 * Returns 1 on success, 0 on duplicate phone or validation error, -1 on allocation failure.
 */
int addContact(Contact **head, int *nextId, const char *name, const char *phone, const char *email);

/**
 * Displays all contacts in console format (for CLI and terminal mode).
 */
void displayContacts(const Contact *head);

/**
 * Searches contacts by query (case-insensitive substring match on name, exact/prefix on phone or ID).
 * Fills the 'results' array with pointers to matching nodes.
 * Returns count of matching contacts.
 */
int searchContacts(Contact *head, const char *query, Contact **results, int maxResults);

/**
 * Finds a single contact by unique ID.
 * Returns pointer to Contact if found, NULL otherwise.
 */
Contact* findContactById(Contact *head, int id);

/**
 * Checks if a phone number already exists in the list (excluding a specific contact id).
 * Returns 1 if phone exists, 0 otherwise.
 */
int isPhoneDuplicate(const Contact *head, const char *phone, int excludeId);

/**
 * Updates an existing contact's details by ID.
 * Validates fields and verifies no other contact has the new phone number.
 * Returns 1 on success, 0 on validation/duplicate error, -1 if contact not found.
 */
int updateContact(Contact *head, int id, const char *name, const char *phone, const char *email);

/**
 * Deletes a contact by ID from the singly linked list.
 * Correctly handles:
 *  - Deleting head node (first node)
 *  - Deleting intermediate node (middle)
 *  - Deleting tail node (last)
 *  - Deleting the only remaining node
 * Releases memory using free().
 * Returns 1 on success, 0 if contact not found.
 */
int deleteContact(Contact **head, int id);

/**
 * Sorts the singly linked list alphabetically by contact name.
 * Implemented in C using Bubble Sort on the linked list (swapping node payload data).
 */
void sortContacts(Contact **head);

/**
 * Returns total number of nodes in the linked list.
 */
int getContactCount(const Contact *head);

/**
 * Traverses the linked list and frees all allocated nodes.
 * Sets *head to NULL.
 */
void freeContacts(Contact **head);

#endif /* CONTACT_H */

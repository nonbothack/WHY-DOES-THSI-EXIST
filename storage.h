#ifndef STORAGE_H
#define STORAGE_H

#include "contact.h"

#define DEFAULT_STORAGE_FILE "contacts.csv"

/**
 * Loads contacts from a CSV file into the singly linked list.
 * Safely handles missing, empty, or corrupted files.
 * Updates nextId to (highest_loaded_id + 1).
 * Returns the count of contacts successfully loaded.
 */
int loadContacts(Contact **head, const char *filename, int *nextId);

/**
 * Saves all contacts from the singly linked list to a CSV file.
 * Returns 1 on success, 0 on file open/write error.
 */
int saveContacts(const Contact *head, const char *filename);

/**
 * Populates sample demo contacts if list is empty.
 * Sets nextId appropriately.
 */
void loadDemoContacts(Contact **head, int *nextId);

#endif /* STORAGE_H */

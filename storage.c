#include "storage.h"

/**
 * Loads contacts from the persistent CSV file.
 * Handles missing file, empty lines, and header row gracefully.
 */
int loadContacts(Contact **head, const char *filename, int *nextId) {
    if (!head || !filename) return 0;

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        /* File doesn't exist yet - perfectly normal on first run */
        if (nextId) *nextId = 1;
        return 0;
    }

    char line[256];
    int count = 0;
    int maxId = 0;

    /* Read line by line */
    while (fgets(line, sizeof(line), file)) {
        /* Remove trailing newline / carriage return */
        line[strcspn(line, "\r\n")] = '\0';

        /* Skip empty lines */
        if (strlen(line) == 0) continue;

        /* Skip CSV header if present */
        if (strncmp(line, "ID,", 3) == 0 || strncmp(line, "id,", 3) == 0) {
            continue;
        }

        /* Parse CSV columns: id, name, phone, email */
        int id = 0;
        char name[50] = {0};
        char phone[20] = {0};
        char email[60] = {0};

        /* Custom CSV field tokenizer for safety */
        char *token = strtok(line, ",");
        if (token) id = atoi(token);

        token = strtok(NULL, ",");
        if (token) strncpy(name, token, sizeof(name) - 1);

        token = strtok(NULL, ",");
        if (token) strncpy(phone, token, sizeof(phone) - 1);

        token = strtok(NULL, ",");
        if (token) strncpy(email, token, sizeof(email) - 1);

        if (id > 0 && strlen(name) > 0 && strlen(phone) > 0) {
            Contact *newNode = createNode(id, name, phone, email);
            if (newNode != NULL) {
                if (*head == NULL) {
                    *head = newNode;
                } else {
                    Contact *temp = *head;
                    while (temp->next != NULL) {
                        temp = temp->next;
                    }
                    temp->next = newNode;
                }
                count++;
                if (id > maxId) {
                    maxId = id;
                }
            }
        }
    }

    fclose(file);

    if (nextId) {
        *nextId = maxId + 1;
    }

    return count;
}

/**
 * Saves all contacts in the singly linked list to the CSV file.
 */
int saveContacts(const Contact *head, const char *filename) {
    if (!filename) return 0;

    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Failed to open storage file for writing");
        return 0;
    }

    /* Write CSV header */
    fprintf(file, "ID,Name,Phone,Email\n");

    const Contact *current = head;
    while (current != NULL) {
        fprintf(file, "%d,%s,%s,%s\n",
                current->id,
                current->name,
                current->phone,
                current->email);
        current = current->next;
    }

    fclose(file);
    return 1;
}

/**
 * Populates 5 demo contacts to demonstrate the system immediately.
 * Labeled clearly as [Demo] data as per specification.
 */
void loadDemoContacts(Contact **head, int *nextId) {
    if (!head || *head != NULL) return; /* Only load if list is empty */

    int id = 1;
    addContact(head, &id, "Aarav Sharma", "09820112345", "aarav.sharma@example.com");
    addContact(head, &id, "Priya Patel", "09819234567", "priya.patel@example.com");
    addContact(head, &id, "Rohan Mehta", "09820345678", "rohan.mehta@example.com");
    addContact(head, &id, "Ananya Deshmukh", "09819456789", "ananya.d@example.com");
    addContact(head, &id, "Vikram Iyer", "09820567890", "vikram.iyer@example.com");

    if (nextId) {
        *nextId = id;
    }
}

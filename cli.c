#include "cli.h"

static void readLine(char *buffer, int size) {
    if (fgets(buffer, size, stdin) != NULL) {
        buffer[strcspn(buffer, "\r\n")] = '\0';
    } else {
        buffer[0] = '\0';
    }
}

void runCliMenu(Contact **head, int *nextId, const char *storageFile) {
    int choice = 0;
    char inputBuffer[128];

    while (1) {
        printf("\n======================================================\n");
        printf("              CONTACT MANAGEMENT SYSTEM                \n");
        printf("        CORE ENGINE: DYNAMIC SINGLY LINKED LIST        \n");
        printf("======================================================\n");
        printf(" [Total Contacts in Memory: %d]\n", getContactCount(*head));
        printf("------------------------------------------------------\n");
        printf(" 1. Add New Contact\n");
        printf(" 2. Display All Contacts (Traverse List)\n");
        printf(" 3. Search Contacts (by Name, Phone, or ID)\n");
        printf(" 4. Update Contact Details\n");
        printf(" 5. Delete Contact by ID (with confirmation)\n");
        printf(" 6. Sort Contacts Alphabetically (Bubble Sort in C)\n");
        printf(" 7. Seed Sample Contacts (if empty)\n");
        printf(" 8. Save & Exit\n");
        printf("======================================================\n");
        printf(" Enter your choice (1-8): ");

        if (!fgets(inputBuffer, sizeof(inputBuffer), stdin)) {
            break;
        }

        choice = atoi(inputBuffer);

        switch (choice) {
            case 1: {
                char name[50], phone[20], email[60];
                printf("\n--- Add New Contact ---\n");
                printf("Enter Full Name: ");
                readLine(name, sizeof(name));
                printf("Enter Phone Number (strings preserve leading 0s): ");
                readLine(phone, sizeof(phone));
                printf("Enter Email Address: ");
                readLine(email, sizeof(email));

                int result = addContact(head, nextId, name, phone, email);
                if (result > 0) {
                    saveContacts(*head, storageFile);
                    printf("[SUCCESS] Contact added successfully with ID: %d!\n", result);
                } else if (result == 0) {
                    printf("[ERROR] Failed: Missing required fields or Duplicate phone number '%s'!\n", phone);
                } else {
                    printf("[ERROR] Memory allocation failure!\n");
                }
                break;
            }
            case 2: {
                printf("\n--- All Contacts (Singly Linked List Traversal) ---");
                displayContacts(*head);
                break;
            }
            case 3: {
                char query[64];
                printf("\nEnter Search Term (Name / Phone / ID): ");
                readLine(query, sizeof(query));

                Contact *results[100];
                int found = searchContacts(*head, query, results, 100);

                if (found == 0) {
                    printf("[INFO] No contacts matching '%s' found.\n", query);
                } else {
                    printf("\n[Found %d match(es)]\n", found);
                    printf("+-----+--------------------------------+--------------------+--------------------------------+\n");
                    printf("| %-3s | %-30s | %-18s | %-30s |\n", "ID", "Name", "Phone", "Email");
                    printf("+-----+--------------------------------+--------------------+--------------------------------+\n");
                    for (int i = 0; i < found; i++) {
                        printf("| %-3d | %-30s | %-18s | %-30s |\n",
                               results[i]->id, results[i]->name, results[i]->phone, results[i]->email);
                    }
                    printf("+-----+--------------------------------+--------------------+--------------------------------+\n");
                }
                break;
            }
            case 4: {
                printf("\nEnter Contact ID to update: ");
                readLine(inputBuffer, sizeof(inputBuffer));
                int id = atoi(inputBuffer);

                Contact *existing = findContactById(*head, id);
                if (!existing) {
                    printf("[ERROR] Contact with ID %d does not exist.\n", id);
                    break;
                }

                printf("Found: %s (Phone: %s, Email: %s)\n", existing->name, existing->phone, existing->email);

                char name[50], phone[20], email[60];
                printf("Enter New Name [Press ENTER to keep '%s']: ", existing->name);
                readLine(name, sizeof(name));
                if (strlen(name) == 0) strncpy(name, existing->name, sizeof(name));

                printf("Enter New Phone [Press ENTER to keep '%s']: ", existing->phone);
                readLine(phone, sizeof(phone));
                if (strlen(phone) == 0) strncpy(phone, existing->phone, sizeof(phone));

                printf("Enter New Email [Press ENTER to keep '%s']: ", existing->email);
                readLine(email, sizeof(email));
                if (strlen(email) == 0) strncpy(email, existing->email, sizeof(email));

                int updated = updateContact(*head, id, name, phone, email);
                if (updated == 1) {
                    saveContacts(*head, storageFile);
                    printf("[SUCCESS] Contact ID %d updated successfully!\n", id);
                } else if (updated == 0) {
                    printf("[ERROR] Update rejected: Duplicate phone number or invalid fields.\n");
                } else {
                    printf("[ERROR] Contact not found.\n");
                }
                break;
            }
            case 5: {
                printf("\nEnter Contact ID to delete: ");
                readLine(inputBuffer, sizeof(inputBuffer));
                int id = atoi(inputBuffer);

                Contact *existing = findContactById(*head, id);
                if (!existing) {
                    printf("[ERROR] Contact with ID %d not found.\n", id);
                    break;
                }

                printf("Are you sure you want to delete '%s' (ID: %d)? (y/N): ", existing->name, id);
                readLine(inputBuffer, sizeof(inputBuffer));
                if (inputBuffer[0] == 'y' || inputBuffer[0] == 'Y') {
                    int deleted = deleteContact(head, id);
                    if (deleted) {
                        saveContacts(*head, storageFile);
                        printf("[SUCCESS] Contact with ID %d was deleted from the linked list.\n", id);
                    } else {
                        printf("[ERROR] Deletion failed.\n");
                    }
                } else {
                    printf("[INFO] Deletion cancelled.\n");
                }
                break;
            }
            case 6: {
                printf("\nSorting contacts alphabetically by name in C...");
                sortContacts(head);
                saveContacts(*head, storageFile);
                printf("\n[SUCCESS] Linked list sorted! Displaying sorted list:\n");
                displayContacts(*head);
                break;
            }
            case 7: {
                if (*head != NULL) {
                    printf("[INFO] Contacts list is not empty (%d contacts exist). Demo data skipped.\n", getContactCount(*head));
                } else {
                    loadDemoContacts(head, nextId);
                    saveContacts(*head, storageFile);
                    printf("[SUCCESS] Loaded 5 sample demo contacts into the linked list!\n");
                    displayContacts(*head);
                }
                break;
            }
            case 8: {
                printf("\nSaving contacts to '%s' and releasing dynamically allocated memory...\n", storageFile);
                saveContacts(*head, storageFile);
                freeContacts(head);
                printf("[OK] Memory cleanly deallocated. Thank you! Exiting.\n");
                return;
            }
            default:
                printf("[ERROR] Invalid selection '%d'. Please choose between 1 and 8.\n", choice);
                break;
        }
    }
}

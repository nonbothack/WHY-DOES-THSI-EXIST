#include "contact.h"

/* Case-insensitive string search helper for portable C */
static int containsIgnoreCase(const char *haystack, const char *needle) {
    if (!haystack || !needle) return 0;
    if (*needle == '\0') return 1;

    for (; *haystack != '\0'; haystack++) {
        const char *h = haystack;
        const char *n = needle;
        while (*h && *n && (tolower((unsigned char)*h) == tolower((unsigned char)*n))) {
            h++;
            n++;
        }
        if (*n == '\0') {
            return 1; /* Match found */
        }
    }
    return 0;
}

/* Trim leading and trailing whitespace helper */
static void sanitizeString(char *dest, const char *src, size_t maxLen) {
    if (!src || !dest || maxLen == 0) return;

    /* Skip leading whitespace */
    while (*src && isspace((unsigned char)*src)) src++;

    size_t i = 0;
    while (*src && i < maxLen - 1) {
        dest[i++] = *src++;
    }
    dest[i] = '\0';

    /* Trim trailing whitespace */
    while (i > 0 && isspace((unsigned char)dest[i - 1])) {
        dest[--i] = '\0';
    }
}

/**
 * Creates and allocates a new Contact node using malloc().
 */
Contact* createNode(int id, const char *name, const char *phone, const char *email) {
    Contact *newNode = (Contact*)malloc(sizeof(Contact));
    if (newNode == NULL) {
        perror("Memory allocation failed for Contact node");
        return NULL;
    }

    newNode->id = id;
    sanitizeString(newNode->name, name, sizeof(newNode->name));
    sanitizeString(newNode->phone, phone, sizeof(newNode->phone));
    sanitizeString(newNode->email, email, sizeof(newNode->email));
    newNode->next = NULL;

    return newNode;
}

/**
 * Checks if a phone number already exists in the linked list.
 * If excludeId > 0, skips checking against that specific contact (useful during update).
 */
int isPhoneDuplicate(const Contact *head, const char *phone, int excludeId) {
    if (!phone || strlen(phone) == 0) return 0;

    const Contact *current = head;
    while (current != NULL) {
        if (current->id != excludeId && strcmp(current->phone, phone) == 0) {
            return 1; /* Duplicate found */
        }
        current = current->next;
    }
    return 0;
}

/**
 * Adds a contact to the end of the singly linked list.
 */
int addContact(Contact **head, int *nextId, const char *name, const char *phone, const char *email) {
    if (!head || !name || !phone || !email) return 0;

    /* Field validations */
    if (strlen(name) == 0 || strlen(phone) == 0) {
        return 0; /* Required fields missing */
    }

    /* Check for duplicate phone number */
    if (isPhoneDuplicate(*head, phone, -1)) {
        return 0; /* Duplicate phone rejected */
    }

    int assignedId = (nextId != NULL && *nextId > 0) ? (*nextId)++ : 1;
    Contact *newNode = createNode(assignedId, name, phone, email);
    if (newNode == NULL) {
        return -1; /* Allocation failure */
    }

    /* If list is currently empty, new node becomes the head */
    if (*head == NULL) {
        *head = newNode;
        return assignedId;
    }

    /* Otherwise traverse to the tail node and append */
    Contact *current = *head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = newNode;

    return assignedId;
}

/**
 * Displays all contacts in console format (for CLI and terminal mode).
 */
void displayContacts(const Contact *head) {
    if (head == NULL) {
        printf("\n[Empty State] No contacts found in the system.\n");
        return;
    }

    printf("\n+-----+--------------------------------+--------------------+--------------------------------+\n");
    printf("| %-3s | %-30s | %-18s | %-30s |\n", "ID", "Name", "Phone", "Email");
    printf("+-----+--------------------------------+--------------------+--------------------------------+\n");

    const Contact *current = head;
    while (current != NULL) {
        printf("| %-3d | %-30s | %-18s | %-30s |\n",
               current->id, current->name, current->phone, current->email);
        current = current->next;
    }

    printf("+-----+--------------------------------+--------------------+--------------------------------+\n");
}

/**
 * Searches contacts by query (case-insensitive substring match on name, phone, or ID).
 */
int searchContacts(Contact *head, const char *query, Contact **results, int maxResults) {
    if (!head || !query || !results || maxResults <= 0) return 0;

    int matchCount = 0;
    char idStr[16];
    Contact *current = head;

    while (current != NULL && matchCount < maxResults) {
        snprintf(idStr, sizeof(idStr), "%d", current->id);

        /* Case-insensitive search on name, phone, or exact/substring on ID */
        if (containsIgnoreCase(current->name, query) ||
            containsIgnoreCase(current->phone, query) ||
            containsIgnoreCase(current->email, query) ||
            strcmp(idStr, query) == 0) {
            results[matchCount++] = current;
        }
        current = current->next;
    }

    return matchCount;
}

/**
 * Finds a single contact by unique ID.
 */
Contact* findContactById(Contact *head, int id) {
    Contact *current = head;
    while (current != NULL) {
        if (current->id == id) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/**
 * Updates an existing contact's details by ID.
 */
int updateContact(Contact *head, int id, const char *name, const char *phone, const char *email) {
    Contact *target = findContactById(head, id);
    if (target == NULL) {
        return -1; /* Contact not found */
    }

    /* Validations */
    if (!name || strlen(name) == 0 || !phone || strlen(phone) == 0) {
        return 0; /* Invalid inputs */
    }

    /* Check duplicate phone excluding this contact's own ID */
    if (isPhoneDuplicate(head, phone, id)) {
        return 0; /* Phone number already in use by another contact */
    }

    sanitizeString(target->name, name, sizeof(target->name));
    sanitizeString(target->phone, phone, sizeof(target->phone));
    sanitizeString(target->email, email ? email : "", sizeof(target->email));

    return 1; /* Success */
}

/**
 * Deletes a contact by ID from the singly linked list.
 * Safely handles:
 *  Case 1: Empty list
 *  Case 2: Node is the head node (including if it is the only node)
 *  Case 3: Node is in the middle or at the end
 */
int deleteContact(Contact **head, int id) {
    if (head == NULL || *head == NULL) {
        return 0; /* Empty list */
    }

    /* Case 2: Contact is the head node */
    if ((*head)->id == id) {
        Contact *nodeToDelete = *head;
        *head = (*head)->next; /* Head moves to next node (or NULL if it was the only node) */
        free(nodeToDelete);
        return 1;
    }

    /* Case 3: Search for contact in subsequent nodes */
    Contact *prev = *head;
    Contact *curr = (*head)->next;

    while (curr != NULL && curr->id != id) {
        prev = curr;
        curr = curr->next;
    }

    if (curr == NULL) {
        return 0; /* Contact with given ID not found */
    }

    /* Unlink current node and release its memory */
    prev->next = curr->next;
    free(curr);

    return 1;
}

/**
 * Sorts the singly linked list alphabetically by contact name.
 * Uses standard Bubble Sort on the linked list by swapping node payload data.
 * Ideal for academic demonstration and technical explanation.
 */
void sortContacts(Contact **head) {
    if (head == NULL || *head == NULL || (*head)->next == NULL) {
        return; /* List has 0 or 1 element, already sorted */
    }

    int swapped;
    Contact *ptr1;
    Contact *lptr = NULL; /* Marks the end of sorted portion */

    do {
        swapped = 0;
        ptr1 = *head;

        while (ptr1->next != lptr) {
            /* Case-insensitive string comparison */
            if (strcasecmp(ptr1->name, ptr1->next->name) > 0) {
                /* Swap data payload between ptr1 and ptr1->next */
                int tempId = ptr1->id;
                char tempName[50];
                char tempPhone[20];
                char tempEmail[60];

                strncpy(tempName, ptr1->name, sizeof(tempName));
                strncpy(tempPhone, ptr1->phone, sizeof(tempPhone));
                strncpy(tempEmail, ptr1->email, sizeof(tempEmail));

                ptr1->id = ptr1->next->id;
                strncpy(ptr1->name, ptr1->next->name, sizeof(ptr1->name));
                strncpy(ptr1->phone, ptr1->next->phone, sizeof(ptr1->phone));
                strncpy(ptr1->email, ptr1->next->email, sizeof(ptr1->email));

                ptr1->next->id = tempId;
                strncpy(ptr1->next->name, tempName, sizeof(ptr1->next->name));
                strncpy(ptr1->next->phone, tempPhone, sizeof(ptr1->next->phone));
                strncpy(ptr1->next->email, tempEmail, sizeof(ptr1->next->email));

                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
        lptr = ptr1;
    } while (swapped);
}

/**
 * Returns total number of nodes in the linked list.
 */
int getContactCount(const Contact *head) {
    int count = 0;
    const Contact *current = head;
    while (current != NULL) {
        count++;
        current = current->next;
    }
    return count;
}

/**
 * Traverses the linked list and frees all allocated nodes.
 */
void freeContacts(Contact **head) {
    if (head == NULL) return;

    Contact *current = *head;
    Contact *nextNode = NULL;

    while (current != NULL) {
        nextNode = current->next;
        free(current);
        current = nextNode;
    }

    *head = NULL;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "contact.h"
#include "storage.h"

#define TEST_FILE "test_contacts.csv"

static int g_testsPassed = 0;
static int g_testsTotal = 0;

#define RUN_TEST(fn) do { \
    g_testsTotal++; \
    printf("Running %-50s ... ", #fn); \
    if (fn()) { \
        printf("[PASS]\n"); \
        g_testsPassed++; \
    } else { \
        printf("[FAIL]\n"); \
    } \
} while(0)

/* Test 1: Adding multiple contacts to linked list */
static int test_add_multiple() {
    Contact *head = NULL;
    int nextId = 1;

    int id1 = addContact(&head, &nextId, "Rohan Sharma", "09820011111", "rohan@example.com");
    int id2 = addContact(&head, &nextId, "Pooja Verma", "09820022222", "pooja@example.com");
    int id3 = addContact(&head, &nextId, "Amit Kulkarni", "09820033333", "amit@example.com");

    if (id1 != 1 || id2 != 2 || id3 != 3) {
        freeContacts(&head);
        return 0;
    }

    if (getContactCount(head) != 3) {
        freeContacts(&head);
        return 0;
    }

    freeContacts(&head);
    return (head == NULL);
}

/* Test 2: Rejecting duplicate phone numbers */
static int test_duplicate_phone() {
    Contact *head = NULL;
    int nextId = 1;

    addContact(&head, &nextId, "Aarav", "09800000001", "aarav@test.com");
    int dup = addContact(&head, &nextId, "Duplicate User", "09800000001", "other@test.com");

    if (dup != 0) {
        freeContacts(&head);
        return 0; /* Should have rejected duplicate phone */
    }

    if (getContactCount(head) != 1) {
        freeContacts(&head);
        return 0;
    }

    freeContacts(&head);
    return 1;
}

/* Test 3: Search for existing and non-existent contacts */
static int test_search() {
    Contact *head = NULL;
    int nextId = 1;

    addContact(&head, &nextId, "Sachin Tendulkar", "09999999991", "sachin@cricket.org");
    addContact(&head, &nextId, "Rohit Sharma", "09999999992", "rohit@cricket.org");

    Contact *results[10];

    /* Search by case-insensitive name */
    int count1 = searchContacts(head, "sachin", results, 10);
    if (count1 != 1 || strcmp(results[0]->name, "Sachin Tendulkar") != 0) {
        freeContacts(&head);
        return 0;
    }

    /* Search by phone number */
    int count2 = searchContacts(head, "09999999992", results, 10);
    if (count2 != 1 || strcmp(results[0]->name, "Rohit Sharma") != 0) {
        freeContacts(&head);
        return 0;
    }

    /* Search for non-existent */
    int count3 = searchContacts(head, "NonExistentPerson", results, 10);
    if (count3 != 0) {
        freeContacts(&head);
        return 0;
    }

    freeContacts(&head);
    return 1;
}

/* Test 4: Update contact details and prevent duplicate phone during update */
static int test_update() {
    Contact *head = NULL;
    int nextId = 1;

    addContact(&head, &nextId, "Neha", "09111111111", "neha@test.com");
    addContact(&head, &nextId, "Kavita", "09222222222", "kavita@test.com");

    /* Update Neha's email */
    int res1 = updateContact(head, 1, "Neha Sen", "09111111111", "nehasen@newdomain.com");
    if (res1 != 1) {
        freeContacts(&head);
        return 0;
    }

    Contact *c = findContactById(head, 1);
    if (!c || strcmp(c->name, "Neha Sen") != 0 || strcmp(c->email, "nehasen@newdomain.com") != 0) {
        freeContacts(&head);
        return 0;
    }

    /* Attempt updating Kavita with Neha's phone number -> Should fail */
    int res2 = updateContact(head, 2, "Kavita", "09111111111", "kavita@test.com");
    if (res2 != 0) {
        freeContacts(&head);
        return 0; /* Duplicate phone wasn't blocked! */
    }

    freeContacts(&head);
    return 1;
}

/* Test 5: Delete first node (Head), middle node, and last node (Tail) */
static int test_delete_head_middle_tail() {
    Contact *head = NULL;
    int nextId = 1;

    addContact(&head, &nextId, "Node 1", "09000000001", "n1@t.com"); /* ID 1 */
    addContact(&head, &nextId, "Node 2", "09000000002", "n2@t.com"); /* ID 2 */
    addContact(&head, &nextId, "Node 3", "09000000003", "n3@t.com"); /* ID 3 */
    addContact(&head, &nextId, "Node 4", "09000000004", "n4@t.com"); /* ID 4 */

    /* 1. Delete middle node (ID 2) */
    int delMiddle = deleteContact(&head, 2);
    if (!delMiddle || getContactCount(head) != 3 || findContactById(head, 2) != NULL) {
        freeContacts(&head);
        return 0;
    }

    /* 2. Delete head node (ID 1) */
    int delHead = deleteContact(&head, 1);
    if (!delHead || getContactCount(head) != 2 || head->id != 3) {
        freeContacts(&head);
        return 0;
    }

    /* 3. Delete tail node (ID 4) */
    int delTail = deleteContact(&head, 4);
    if (!delTail || getContactCount(head) != 1 || head->id != 3 || head->next != NULL) {
        freeContacts(&head);
        return 0;
    }

    freeContacts(&head);
    return 1;
}

/* Test 6: Delete the only node in list */
static int test_delete_only_node() {
    Contact *head = NULL;
    int nextId = 1;

    addContact(&head, &nextId, "Solo Contact", "09777777777", "solo@test.com");
    if (getContactCount(head) != 1) {
        freeContacts(&head);
        return 0;
    }

    int del = deleteContact(&head, 1);
    if (!del || head != NULL || getContactCount(head) != 0) {
        freeContacts(&head);
        return 0;
    }

    return 1;
}

/* Test 7: Sort contacts alphabetically in C */
static int test_sort_alphabetical() {
    Contact *head = NULL;
    int nextId = 1;

    addContact(&head, &nextId, "Zoya Akhtar", "09333333331", "zoya@film.in");
    addContact(&head, &nextId, "Anil Kapoor", "09333333332", "anil@film.in");
    addContact(&head, &nextId, "Manish Malhotra", "09333333333", "manish@fashion.in");

    sortContacts(&head);

    Contact *curr = head;
    if (strcmp(curr->name, "Anil Kapoor") != 0) { freeContacts(&head); return 0; }
    curr = curr->next;
    if (strcmp(curr->name, "Manish Malhotra") != 0) { freeContacts(&head); return 0; }
    curr = curr->next;
    if (strcmp(curr->name, "Zoya Akhtar") != 0) { freeContacts(&head); return 0; }

    freeContacts(&head);
    return 1;
}

/* Test 8: File persistence (save and reload) */
static int test_file_persistence() {
    Contact *head1 = NULL;
    int nextId1 = 1;

    addContact(&head1, &nextId1, "Vikram", "09444444441", "vikram@test.com");
    addContact(&head1, &nextId1, "Deepa", "09444444442", "deepa@test.com");

    int saved = saveContacts(head1, TEST_FILE);
    freeContacts(&head1);
    if (!saved) return 0;

    /* Reload into new linked list */
    Contact *head2 = NULL;
    int nextId2 = 1;
    int loaded = loadContacts(&head2, TEST_FILE, &nextId2);

    if (loaded != 2 || nextId2 != 3) {
        freeContacts(&head2);
        remove(TEST_FILE);
        return 0;
    }

    Contact *c1 = findContactById(head2, 1);
    Contact *c2 = findContactById(head2, 2);

    if (!c1 || !c2 || strcmp(c1->name, "Vikram") != 0 || strcmp(c2->name, "Deepa") != 0) {
        freeContacts(&head2);
        remove(TEST_FILE);
        return 0;
    }

    freeContacts(&head2);
    remove(TEST_FILE);
    return 1;
}

/* Test 9: Handling empty list and invalid inputs */
static int test_empty_and_invalid() {
    Contact *head = NULL;
    int nextId = 1;

    /* Delete from empty list */
    if (deleteContact(&head, 999) != 0) return 0;

    /* Find in empty list */
    if (findContactById(head, 1) != NULL) return 0;

    /* Search in empty list */
    Contact *results[5];
    if (searchContacts(head, "query", results, 5) != 0) return 0;

    /* Sort empty list without crashing */
    sortContacts(&head);

    /* Add with missing name */
    if (addContact(&head, &nextId, "", "09555555555", "email@t.com") != 0) return 0;

    /* Add with missing phone */
    if (addContact(&head, &nextId, "Name", "", "email@t.com") != 0) return 0;

    return 1;
}

int main() {
    printf("====================================================\n");
    printf("   DATA STRUCTURES UNIT TESTS: SINGLY LINKED LIST   \n");
    printf("====================================================\n");

    RUN_TEST(test_add_multiple);
    RUN_TEST(test_duplicate_phone);
    RUN_TEST(test_search);
    RUN_TEST(test_update);
    RUN_TEST(test_delete_head_middle_tail);
    RUN_TEST(test_delete_only_node);
    RUN_TEST(test_sort_alphabetical);
    RUN_TEST(test_file_persistence);
    RUN_TEST(test_empty_and_invalid);

    printf("====================================================\n");
    printf(" Test Results: %d/%d passed\n", g_testsPassed, g_testsTotal);
    printf("====================================================\n");

    return (g_testsPassed == g_testsTotal) ? 0 : 1;
}

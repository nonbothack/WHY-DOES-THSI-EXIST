#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contact.h"
#include "storage.h"
#include "server.h"
#include "cli.h"

#define DEFAULT_PORT 8080

static void printUsage(const char *progName) {
    printf("Usage: %s [OPTIONS]\n", progName);
    printf("Options:\n");
    printf("  --cli, -c            Launch interactive terminal CLI mode \n");
    printf("  --port, -p <PORT>    Specify HTTP port for Web GUI mode (default: %d)\n", DEFAULT_PORT);
    printf("  --file, -f <FILE>    Specify CSV storage file (default: %s)\n", DEFAULT_STORAGE_FILE);
    printf("  --demo               Preload demo contacts if list is empty\n");
    printf("  --help, -h           Show this help message\n");
}

int main(int argc, char *argv[]) {
    int cliMode = 0;
    int port = DEFAULT_PORT;
    const char *storageFile = DEFAULT_STORAGE_FILE;
    int forceDemo = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--cli") == 0 || strcmp(argv[i], "-c") == 0) {
            cliMode = 1;
        } else if ((strcmp(argv[i], "--port") == 0 || strcmp(argv[i], "-p") == 0) && i + 1 < argc) {
            port = atoi(argv[++i]);
            if (port <= 0 || port > 65535) port = DEFAULT_PORT;
        } else if ((strcmp(argv[i], "--file") == 0 || strcmp(argv[i], "-f") == 0) && i + 1 < argc) {
            storageFile = argv[++i];
        } else if (strcmp(argv[i], "--demo") == 0) {
            forceDemo = 1;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printUsage(argv[0]);
            return 0;
        }
    }

    printf("\n======================================================\n");
    printf("              CONTACT MANAGEMENT SYSTEM                \n");
    printf("        CORE ENGINE: DYNAMIC SINGLY LINKED LIST        \n");
    printf("======================================================\n");

    /* Initialize Head pointer of Singly Linked List */
    Contact *head = NULL;
    int nextId = 1;

    /* Load existing contacts from disk */
    int loaded = loadContacts(&head, storageFile, &nextId);
    printf("[Storage] Loaded %d contact(s) from '%s'.\n", loaded, storageFile);

    /* If list is empty and user requested demo data or first initialization */
    if (loaded == 0 && forceDemo) {
        loadDemoContacts(&head, &nextId);
        saveContacts(head, storageFile);
        printf("[Storage] Seeded 5 initial demo contacts.\n");
    }

    if (cliMode) {
        /* Run terminal-based interactive CLI */
        runCliMenu(&head, &nextId, storageFile);
    } else {
        /* Run modern web application server */
        printf("[Server] Initializing Web Interface on port %d...\n", port);
        startServer(port, &head, &nextId, storageFile);

        /* Cleanup after server termination */
        printf("\n[Server] Shutting down. Saving state to '%s'...\n", storageFile);
        saveContacts(head, storageFile);
        freeContacts(&head);
        printf("[Server] Linked list dynamic memory cleanly released. Bye!\n");
    }

    return 0;
}

# Contact Management System Using a Singly Linked List in C

> **Academic Mini-Project**: Data Structures Project  
> **Course**: Data Structures & Algorithms Lab  
> **Primary Data Structure**: Dynamic Singly Linked List (`struct Contact *`)  
> **Core Language**: C (C99 Standard, zero external runtime libraries)  
> **User Interfaces**: Modern Desktop-Style Web GUI (HTML5/Vanilla CSS/ES6 JavaScript) & Interactive Terminal CLI  

---

## 📌 Project Overview

This Contact Management System demonstrates the real-world application of a **Singly Linked List** implemented in C. Unlike typical web projects that store state in JavaScript arrays or external databases, this application performs **100% of its data structure operations** (insertion, linear search, in-place update, node deletion, bubble sort, dynamic memory management) natively in **C** using dynamically allocated heap memory (`malloc()` and `free()`).

The C backend features an integrated lightweight HTTP/REST server using standard POSIX sockets (`sys/socket.h`), enabling a clean graphical web dashboard while persisting all contact records to `contacts.csv`.

---

## 📁 Repository Structure

```text
├── contact.h          # Contact node structure and linked-list function prototypes
├── contact.c          # Core Singly Linked List implementation (Add, Search, Delete, Sort, etc.)
├── storage.h          # File storage function prototypes (load, save, demo seeding)
├── storage.c          # CSV file parsing and serialization
├── server.h           # Lightweight C HTTP server interface
├── server.c           # POSIX socket server, router, JSON parser, and REST API
├── cli.h              # Terminal interface header
├── cli.c              # Interactive CLI menu for terminal-only technical review
├── main.c             # Application entry point (CLI vs Web Server mode selector)
├── test_contact.c     # Automated academic unit test suite covering all 9 test cases
├── Makefile           # GCC build automation script
├── contacts.csv       # Persistent disk storage file (auto-generated)
├── ACADEMIC_REPORT.md # Comprehensive 18-section academic project report & technical report
└── frontend/
    ├── index.html     # Semantic HTML5 desktop dashboard layout
    ├── style.css      # Vanilla CSS design system (Inter font, indigo theme, cards)
    └── app.js         # REST API client & interactive Linked List Memory Visualizer
```

---

## 🚀 Quick Start Guide

### Prerequisites
- **GCC Compiler** (`gcc`) supporting C99 or later.
- **GNU Make** (`make`).
- Standard Linux / macOS / POSIX environment. (Windows users can run via WSL2, MinGW, or MSYS2).

### 1. Build the Application
```bash
make all
```
*Compiles with `-Wall -Wextra -std=c99 -pedantic` with zero warnings.*

### 2. Run in Web GUI Mode (Default)
```bash
make run
# Or directly: ./contact_app --port 8080 --demo
```
Open your browser and navigate to:
👉 **`http://localhost:8080`**

### 3. Run in Terminal CLI Mode (Terminal Console)
If your professor or evaluator asks to see the terminal console without a browser:
```bash
make run-cli
# Or directly: ./contact_app --cli
```

### 4. Run Automated Academic Unit Tests
```bash
make test
```
*Executes all 9 unit test scenarios (multiple additions, duplicate phone rejection, search, update, deletion of head/middle/tail/only node, alphabetical sorting, and file persistence).*

---

## 🧩 Core Singly Linked List Node Design

```c
struct Contact {
    int id;                 /* Unique contact identifier */
    char name[50];          /* Contact Full Name */
    char phone[20];         /* Phone Number (String retains leading zeros) */
    char email[60];         /* Contact Email Address */
    struct Contact *next;   /* Pointer to next node in heap */
};
```

### Memory Allocation & Release
- **Node Creation**: `malloc(sizeof(struct Contact))` allocates dynamic heap memory for each contact.
- **Node Deletion**: `free(node)` immediately deallocates heap memory to prevent memory leaks.
- **Head Management**: `Contact *head` tracks the entry point of the list. When `head == NULL`, the list is empty.

---

## ⚡ Implemented Operations & Complexity

| Operation | C Function | Best Time | Worst Time | Space | Explanation |
|---|---|---|---|---|---|
| **Add Contact** | `addContact()` | $O(1)$ | $O(n)$ | $O(1)$ | Traverses to tail to append, validates duplicate phone. |
| **Search Contact** | `searchContacts()` | $O(1)$ | $O(n)$ | $O(1)$ | Traverses nodes; case-insensitive name, phone, or ID match. |
| **Find by ID** | `findContactById()` | $O(1)$ | $O(n)$ | $O(1)$ | Linear scan until matching ID is encountered. |
| **Update Details** | `updateContact()` | $O(1)$ | $O(n)$ | $O(1)$ | Locates node and updates fields; ensures unique phone. |
| **Delete Node** | `deleteContact()` | $O(1)$ | $O(n)$ | $O(1)$ | Handles head, middle, tail, and solo node unlinking & `free()`. |
| **Sort A-Z** | `sortContacts()` | $O(n)$ | $O(n^2)$ | $O(1)$ | Bubble sort on linked list; swaps payloads without breaking pointers. |
| **Load File** | `loadContacts()` | $O(n)$ | $O(n)$ | $O(n)$ | Parses `contacts.csv` on startup and rebuilds list. |
| **Save File** | `saveContacts()` | $O(n)$ | $O(n)$ | $O(1)$ | Writes current list from head to tail into CSV. |
| **Clean Memory**| `freeContacts()` | $O(n)$ | $O(n)$ | $O(1)$ | Traverses every node, calls `free()`, resets `*head = NULL`. |

---

## 🌐 REST API Endpoints (C POSIX Server)

| Method | Endpoint | Description |
|---|---|---|
| `GET` | `/` | Serves `frontend/index.html` |
| `GET` | `/style.css` | Serves Vanilla CSS styles |
| `GET` | `/app.js` | Serves JavaScript frontend client |
| `GET` | `/api/status` | Returns JSON status of server and linked list |
| `GET` | `/api/contacts` | Returns all contacts as a JSON array |
| `GET` | `/api/contacts/search?q=query` | Searches contacts matching query in C |
| `POST` | `/api/contacts` | Adds new contact to linked list (`{"name","phone","email"}`) |
| `POST` | `/api/contacts/update` | Updates contact details (`{"id","name","phone","email"}`) |
| `POST` | `/api/contacts/delete` | Deletes contact node by ID (`{"id": N}`) |
| `POST` | `/api/contacts/sort` | Sorts linked list alphabetically in C |
| `POST` | `/api/contacts/demo` | Seeds 5 sample academic demo contacts |

---

## 📜 Full Academic Report
For the complete 18-section Data Structures Project term-work submission report with flowcharts and step-by-step algorithms, see [ACADEMIC_REPORT.md](file:///home/cranialnerve/Pictures/Trash1/ACADEMIC_REPORT.md).

/**
 * NexusContact - Vercel Serverless Function Bridge
 * Mirror implementation of C Singly Linked List for Serverless Deployments
 */

class ContactNode {
    constructor(id, name, phone, email) {
        this.id = id;
        this.name = name;
        this.phone = phone;
        this.email = email || "";
        this.next = null;
    }
}

class SinglyLinkedList {
    constructor() {
        this.head = null;
        this.nextId = 1;
        this.initDefaultRecords();
    }

    initDefaultRecords() {
        const defaults = [
            { name: "Aarav Sharma", phone: "09820112345", email: "aarav.sharma@example.com" },
            { name: "Ananya Deshmukh", phone: "09819456789", email: "ananya.d@example.com" },
            { name: "Priya Patel", phone: "09819234567", email: "priya.patel@example.com" },
            { name: "Rohan Mehta", phone: "09820345678", email: "rohan.mehta@example.com" },
            { name: "Vikram Iyer", phone: "09820567890", email: "vikram.iyer@example.com" }
        ];

        defaults.forEach(d => {
            this.add(d.name, d.phone, d.email);
        });
    }

    toArray() {
        const results = [];
        let curr = this.head;
        while (curr) {
            results.push({
                id: curr.id,
                name: curr.name,
                phone: curr.phone,
                email: curr.email
            });
            curr = curr.next;
        }
        return results;
    }

    count() {
        let cnt = 0;
        let curr = this.head;
        while (curr) {
            cnt++;
            curr = curr.next;
        }
        return cnt;
    }

    add(name, phone, email) {
        // Check duplicate phone
        let curr = this.head;
        while (curr) {
            if (curr.phone === phone) {
                return { success: false, message: "Duplicate phone number detected!" };
            }
            curr = curr.next;
        }

        const id = this.nextId++;
        const newNode = new ContactNode(id, name, phone, email);

        if (!this.head) {
            this.head = newNode;
        } else {
            let temp = this.head;
            while (temp.next) {
                temp = temp.next;
            }
            temp.next = newNode;
        }

        return { success: true, id: id, message: "Contact added to linked list successfully!" };
    }

    update(id, name, phone, email) {
        let curr = this.head;
        while (curr) {
            if (curr.id === id) {
                curr.name = name;
                curr.phone = phone;
                curr.email = email || "";
                return { success: true, message: "Contact details updated successfully!" };
            }
            curr = curr.next;
        }
        return { success: false, message: "Contact with specified ID not found!" };
    }

    delete(id) {
        if (!this.head) return false;

        if (this.head.id === id) {
            this.head = this.head.next;
            return true;
        }

        let prev = this.head;
        let curr = this.head.next;
        while (curr) {
            if (curr.id === id) {
                prev.next = curr.next;
                return true;
            }
            prev = curr;
            curr = curr.next;
        }
        return false;
    }

    search(query) {
        const q = query.toLowerCase();
        const matches = [];
        let curr = this.head;
        while (curr) {
            if (
                curr.name.toLowerCase().includes(q) ||
                curr.phone.includes(q) ||
                curr.id.toString() === q
            ) {
                matches.push({
                    id: curr.id,
                    name: curr.name,
                    phone: curr.phone,
                    email: curr.email
                });
            }
            curr = curr.next;
        }
        return matches;
    }

    sortAlphabetical() {
        if (!this.head || !this.head.next) return;

        let swapped = true;
        while (swapped) {
            swapped = false;
            let curr = this.head;
            while (curr && curr.next) {
                if (curr.name.localeCompare(curr.next.name, undefined, { sensitivity: 'base' }) > 0) {
                    // In-place payload swap
                    const tmpId = curr.id;
                    const tmpName = curr.name;
                    const tmpPhone = curr.phone;
                    const tmpEmail = curr.email;

                    curr.id = curr.next.id;
                    curr.name = curr.next.name;
                    curr.phone = curr.next.phone;
                    curr.email = curr.next.email;

                    curr.next.id = tmpId;
                    curr.next.name = tmpName;
                    curr.next.phone = tmpPhone;
                    curr.next.email = tmpEmail;

                    swapped = true;
                }
                curr = curr.next;
            }
        }
    }
}

// Global in-memory list instance for warm lambda execution
const sll = new SinglyLinkedList();

module.exports = async (req, res) => {
    // CORS headers
    res.setHeader("Access-Control-Allow-Origin", "*");
    res.setHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS, DELETE");
    res.setHeader("Access-Control-Allow-Headers", "Content-Type");

    if (req.method === "OPTIONS") {
        return res.status(200).end();
    }

    const url = new URL(req.url, `http://${req.headers.host || "localhost"}`);
    const pathname = url.pathname.replace(/^\/api/, "") || "/";
    const method = req.method;

    // Helper to read JSON body
    let body = req.body || {};
    if (typeof body === "string") {
        try { body = JSON.parse(body); } catch (_) {}
    }

    try {
        // 1. Status
        if (pathname === "/status" || pathname === "/status/") {
            return res.status(200).json({
                status: "online",
                totalContacts: sll.count(),
                dataStructure: "Singly Linked List",
                algorithm: "Dynamic Allocation Core"
            });
        }

        // 2. Search contacts
        if (pathname === "/contacts/search" || pathname === "/contacts/search/") {
            const query = url.searchParams.get("q") || "";
            return res.status(200).json(sll.search(query));
        }

        // 3. Sort contacts
        if (pathname === "/contacts/sort" || pathname === "/contacts/sort/") {
            sll.sortAlphabetical();
            return res.status(200).json({
                success: true,
                message: "Contacts sorted alphabetically in Singly Linked List!"
            });
        }

        // 4. Update contact
        if (pathname === "/contacts/update" || pathname === "/contacts/update/") {
            const result = sll.update(parseInt(body.id, 10), body.name, body.phone, body.email);
            return res.status(result.success ? 200 : 400).json(result);
        }

        // 5. Delete contact
        if (pathname === "/contacts/delete" || pathname === "/contacts/delete/") {
            const id = parseInt(body.id, 10) || parseInt(url.searchParams.get("id"), 10);
            if (!id) {
                return res.status(400).json({ success: false, message: "Valid ID required." });
            }
            const deleted = sll.delete(id);
            if (deleted) {
                return res.status(200).json({ success: true, message: "Contact deleted and memory freed." });
            } else {
                return res.status(404).json({ success: false, message: "Contact not found." });
            }
        }

        // 6. Get all contacts OR Add contact
        if (pathname === "/contacts" || pathname === "/contacts/") {
            if (method === "GET") {
                return res.status(200).json(sll.toArray());
            } else if (method === "POST") {
                if (!body.name || !body.phone) {
                    return res.status(400).json({ success: false, message: "Name and Phone are required." });
                }
                const result = sll.add(body.name, body.phone, body.email);
                return res.status(result.success ? 200 : 400).json(result);
            }
        }

        // Fallback 404
        return res.status(404).json({ error: "Endpoint not found", path: pathname });
    } catch (err) {
        return res.status(500).json({ error: "Internal Server Error", message: err.message });
    }
};

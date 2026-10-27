/**
 * NexusContact - Frontend Controller
 * Retro Cyber Terminal Interface for C Singly Linked List Engine
 */

// Global State
let allContacts = [];
let deleteTargetId = null;
let searchDebounceTimer = null;

// Curated Retro Cyber Palettes for Contact Badges / Avatars
const RETRO_PALETTES = [
    { fg: '#B6FF4A', bg: '#122008', border: 'rgba(182, 255, 74, 0.5)', glow: 'rgba(182, 255, 74, 0.2)' },
    { fg: '#39FF88', bg: '#081F12', border: 'rgba(57, 255, 136, 0.5)', glow: 'rgba(57, 255, 136, 0.2)' },
    { fg: '#FFB000', bg: '#231804', border: 'rgba(255, 176, 0, 0.5)', glow: 'rgba(255, 176, 0, 0.2)' },
    { fg: '#00E5FF', bg: '#051E24', border: 'rgba(0, 229, 255, 0.5)', glow: 'rgba(0, 229, 255, 0.2)' },
    { fg: '#FF3CAC', bg: '#250818', border: 'rgba(255, 60, 172, 0.5)', glow: 'rgba(255, 60, 172, 0.2)' },
    { fg: '#FF7A00', bg: '#241205', border: 'rgba(255, 122, 0, 0.5)', glow: 'rgba(255, 122, 0, 0.2)' }
];

// Initialize on DOM Ready
document.addEventListener('DOMContentLoaded', () => {
    fetchContacts();

    // Global Keyboard Shortcuts
    document.addEventListener('keydown', (e) => {
        // Quick Search with '/'
        if (e.key === '/' && document.activeElement.tagName !== 'INPUT') {
            e.preventDefault();
            const searchInput = document.getElementById('input-search');
            if (searchInput) searchInput.focus();
        }
        // Close modals or clear search on Escape
        if (e.key === 'Escape') {
            closeContactModal();
            closeDeleteModal();
            const searchInput = document.getElementById('input-search');
            if (document.activeElement === searchInput) {
                clearSearch();
                searchInput.blur();
            }
        }
    });
});

// Mobile Sidebar Toggle
function toggleSidebar() {
    const sidebar = document.getElementById('sidebar');
    if (sidebar) {
        sidebar.classList.toggle('sidebar-open');
    }
}

// View Navigation Switcher
function switchView(viewName) {
    const views = ['contacts', 'visualizer', 'architecture'];
    views.forEach(v => {
        const container = document.getElementById(`view-${v}-container`);
        const navBtn = document.getElementById(`nav-${v}`);
        if (container) {
            container.style.display = (v === viewName) ? 'block' : 'none';
        }
        if (navBtn) {
            navBtn.classList.toggle('active', v === viewName);
        }
    });

    const titleElem = document.getElementById('view-title');
    const crumbElem = document.getElementById('breadcrumb-current');

    if (viewName === 'contacts') {
        titleElem.textContent = 'CONTACT DATABASE';
        crumbElem.textContent = 'CONTACT DATABASE';
    } else if (viewName === 'visualizer') {
        titleElem.textContent = 'HEAP MEMORY INSPECTOR';
        crumbElem.textContent = 'HEAP INSPECTOR';
        renderVisualizer();
    } else if (viewName === 'architecture') {
        titleElem.textContent = 'SYSTEM ARCHITECTURE & SPEC';
        crumbElem.textContent = 'SYSTEM SPEC & ARCH';
    }

    // Close mobile sidebar on selection
    const sidebar = document.getElementById('sidebar');
    if (sidebar && sidebar.classList.contains('sidebar-open')) {
        sidebar.classList.remove('sidebar-open');
    }
}

// Fetch all contacts from C Singly Linked List backend
async function fetchContacts() {
    try {
        const response = await fetch('/api/contacts');
        if (!response.ok) throw new Error('Failed to fetch from C server');
        allContacts = await response.json();
        renderTable(allContacts);
        updateStats(allContacts);
        renderVisualizer();
    } catch (err) {
        console.error('Fetch error:', err);
        showToast('Unable to connect to C backend engine', 'error');
    }
}

// Render Contact Table in Retro Terminal Style
function renderTable(contacts) {
    const tbody = document.getElementById('contact-tbody');
    const emptyState = document.getElementById('empty-state');
    const table = document.getElementById('contact-table');

    if (!contacts || contacts.length === 0) {
        tbody.innerHTML = '';
        table.style.display = 'none';
        emptyState.style.display = 'block';
        return;
    }

    table.style.display = 'table';
    emptyState.style.display = 'none';

    tbody.innerHTML = contacts.map((c, index) => {
        const cleanName = c.name.replace(/\[Demo\]/gi, '').trim();
        const initials = cleanName.split(' ').map(n => n[0]).join('').substring(0, 2).toUpperCase() || 'C';
        const pal = RETRO_PALETTES[index % RETRO_PALETTES.length];

        return `
            <tr>
                <td>
                    <span class="node-badge font-mono" title="Dynamic heap node index">#${String(c.id).padStart(2, '0')}</span>
                </td>
                <td>
                    <div class="contact-profile">
                        <div class="avatar font-mono" style="color: ${pal.fg}; background: ${pal.bg}; border: 1px solid ${pal.border}; box-shadow: 0 0 10px ${pal.glow};">
                            ${initials}
                        </div>
                        <div class="contact-name-wrap">
                            <span class="contact-name">${escapeHtml(cleanName)}</span>
                            <span class="contact-heap-ref font-mono">PTR: 0x${(0x55a100 + (c.id * 0x140)).toString(16)}</span>
                        </div>
                    </div>
                </td>
                <td>
                    <span class="phone-badge font-mono">${escapeHtml(c.phone)}</span>
                </td>
                <td>
                    ${c.email ? `<span class="email-badge font-mono">${escapeHtml(c.email)}</span>` : '<span class="text-dim font-mono">// UNSET</span>'}
                </td>
                <td style="text-align: right;">
                    <div class="table-actions">
                        <button class="btn-icon" onclick="openEditModal(${c.id})" title="Edit Contact">
                            <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                                <path d="M11 4H4a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2v-7"></path>
                                <path d="M18.5 2.5a2.121 2.121 0 0 1 3 3L12 15l-4 1 1-4 9.5-9.5z"></path>
                            </svg>
                            <span class="font-mono btn-action-label">EDIT</span>
                        </button>
                        <button class="btn-icon delete" onclick="openDeleteModal(${c.id}, '${escapeHtml(cleanName)}')" title="Purge Node">
                            <svg width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                                <polyline points="3 6 5 6 21 6"></polyline>
                                <path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"></path>
                            </svg>
                            <span class="font-mono btn-action-label">PURGE</span>
                        </button>
                    </div>
                </td>
            </tr>
        `;
    }).join('');
}

// Update Dashboard Statistics
function updateStats(contacts) {
    const totalElem = document.getElementById('stat-total-contacts');
    const headElem = document.getElementById('stat-head-pointer');
    const searchElem = document.getElementById('stat-search-results');
    const memoryElem = document.getElementById('stat-memory-usage');

    if (totalElem) {
        totalElem.textContent = String(contacts.length).padStart(2, '0');
    }

    if (searchElem) {
        searchElem.textContent = String(contacts.length).padStart(2, '0');
    }

    // Calculate memory footprint (144 bytes per struct Contact node)
    if (memoryElem) {
        const bytes = contacts.length * 144;
        memoryElem.textContent = `${bytes} B`;
    }

    if (headElem) {
        if (contacts.length > 0) {
            const simulatedHex = '0x' + (0x55a100 + (contacts[0].id * 0x140)).toString(16).toUpperCase();
            headElem.textContent = simulatedHex;
            headElem.style.color = 'var(--retro-amber)';
        } else {
            headElem.textContent = '0x00000000';
            headElem.style.color = 'var(--text-dim)';
        }
    }
}

// Live Search with Debounce (Calls C backend strcasecmp traversal)
function handleSearch(query) {
    const clearBtn = document.getElementById('btn-clear-search');
    clearBtn.style.display = query.trim().length > 0 ? 'block' : 'none';

    clearTimeout(searchDebounceTimer);
    searchDebounceTimer = setTimeout(async () => {
        const trimmed = query.trim();
        if (trimmed === '') {
            renderTable(allContacts);
            updateStats(allContacts);
            return;
        }

        try {
            const res = await fetch(`/api/contacts/search?q=${encodeURIComponent(trimmed)}`);
            if (!res.ok) throw new Error('Search failed');
            const searchResults = await res.json();
            renderTable(searchResults);

            const searchElem = document.getElementById('stat-search-results');
            if (searchElem) searchElem.textContent = String(searchResults.length).padStart(2, '0');
        } catch (err) {
            console.error('Search error:', err);
        }
    }, 150);
}

// Clear Search Input
function clearSearch() {
    const input = document.getElementById('input-search');
    input.value = '';
    document.getElementById('btn-clear-search').style.display = 'none';
    renderTable(allContacts);
    updateStats(allContacts);
}

// Sort Contacts in C (Calls C backend sortContacts Bubble Sort)
async function sortContactsInC() {
    try {
        const res = await fetch('/api/contacts/sort', { method: 'POST' });
        const data = await res.json();
        if (data.success) {
            showToast('Contacts sorted alphabetically in C backend!', 'success');
            await fetchContacts();
        } else {
            showToast(data.message || 'Sorting failed', 'error');
        }
    } catch (err) {
        console.error('Sort error:', err);
        showToast('Error executing C sort operation', 'error');
    }
}

// Open Add Contact Modal
function openAddModal() {
    document.getElementById('modal-title').textContent = 'ALLOCATE NEW NODE';
    document.getElementById('form-contact-id').value = '';
    document.getElementById('form-name').value = '';
    document.getElementById('form-phone').value = '';
    document.getElementById('form-email').value = '';
    document.getElementById('contact-modal').style.display = 'flex';
    setTimeout(() => document.getElementById('form-name').focus(), 50);
}

// Open Edit Contact Modal
function openEditModal(id) {
    const contact = allContacts.find(c => c.id === id);
    if (!contact) return;

    const cleanName = contact.name.replace(/\[Demo\]/gi, '').trim();
    document.getElementById('modal-title').textContent = `UPDATE NODE #${String(contact.id).padStart(2, '0')}`;
    document.getElementById('form-contact-id').value = contact.id;
    document.getElementById('form-name').value = cleanName;
    document.getElementById('form-phone').value = contact.phone;
    document.getElementById('form-email').value = contact.email;
    document.getElementById('contact-modal').style.display = 'flex';
    setTimeout(() => document.getElementById('form-name').focus(), 50);
}

// Close Contact Modal
function closeContactModal() {
    document.getElementById('contact-modal').style.display = 'none';
}

// Handle Add / Edit Form Submission
async function handleFormSubmit(event) {
    event.preventDefault();

    const id = document.getElementById('form-contact-id').value;
    const name = document.getElementById('form-name').value.trim();
    const phone = document.getElementById('form-phone').value.trim();
    const email = document.getElementById('form-email').value.trim();

    if (!name || !phone) {
        showToast('Name and Phone Number are required fields!', 'error');
        return;
    }

    const payload = {
        name: name,
        phone: phone,
        email: email
    };

    try {
        let res;
        if (id) {
            // Update existing contact
            payload.id = parseInt(id, 10);
            res = await fetch('/api/contacts/update', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(payload)
            });
        } else {
            // Add new contact
            res = await fetch('/api/contacts', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(payload)
            });
        }

        const data = await res.json();
        if (data.success) {
            closeContactModal();
            showToast(data.message, 'success');
            await fetchContacts();
        } else {
            showToast(data.message || 'Operation failed', 'error');
        }
    } catch (err) {
        console.error('Form submission error:', err);
        showToast('Error saving contact to C server', 'error');
    }
}

// Open Delete Confirmation Modal
function openDeleteModal(id, name) {
    deleteTargetId = id;
    document.getElementById('delete-contact-name').textContent = name;
    document.getElementById('delete-modal').style.display = 'flex';
}

// Close Delete Modal
function closeDeleteModal() {
    deleteTargetId = null;
    document.getElementById('delete-modal').style.display = 'none';
}

// Confirm and execute C node deletion
async function confirmDelete() {
    if (!deleteTargetId) return;

    try {
        const res = await fetch('/api/contacts/delete', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ id: deleteTargetId })
        });

        const data = await res.json();
        if (data.success) {
            closeDeleteModal();
            showToast('Node purged & 144B heap reclaimed in C!', 'success');
            await fetchContacts();
        } else {
            showToast(data.message || 'Deletion failed', 'error');
        }
    } catch (err) {
        console.error('Delete error:', err);
        showToast('Error communicating with C backend', 'error');
    }
}

// Render Retro Heap Memory Visualizer Console
function renderVisualizer() {
    const chain = document.getElementById('visualizer-chain');
    if (!chain) return;

    if (!allContacts || allContacts.length === 0) {
        chain.innerHTML = `
            <div class="visualizer-empty font-mono">
                <div class="empty-reg-marker head-reg">HEAD_PTR</div>
                <div class="reg-connector">──[►]──</div>
                <div class="empty-reg-marker null-reg">0x00000000 (NULL)</div>
                <div class="empty-hint">// Singly Linked List is empty. 0 heap nodes allocated.</div>
            </div>
        `;
        return;
    }

    const headAddr = '0x' + (0x55a100 + (allContacts[0].id * 0x140)).toString(16).toUpperCase();

    let html = `
        <div class="memory-register head-block">
            <div class="reg-header">HEAD REGISTER</div>
            <div class="reg-body">
                <div class="reg-ptr-val">${headAddr}</div>
                <div class="reg-label">&gt; entry_point</div>
            </div>
        </div>
        <div class="pointer-arrow-connector">
            <div class="arrow-line"></div>
            <div class="arrow-head">►</div>
        </div>
    `;

    allContacts.forEach((c, idx) => {
        const cleanName = c.name.replace(/\[Demo\]/gi, '').trim();
        const baseAddr = 0x55a100 + (c.id * 0x140);
        const nodeAddr = '0x' + baseAddr.toString(16).toUpperCase();
        const nextAddr = (idx < allContacts.length - 1)
            ? '0x' + (0x55a100 + (allContacts[idx + 1].id * 0x140)).toString(16).toUpperCase()
            : '0x00000000';
        const isNullNext = (idx === allContacts.length - 1);

        html += `
            <div class="memory-node-block">
                <div class="node-block-top">
                    <span class="node-block-id">NODE_0x${String(c.id).padStart(2, '0')}</span>
                    <span class="node-block-addr">${nodeAddr}</span>
                </div>
                
                <div class="node-block-payload">
                    <div class="payload-field">
                        <span class="field-offset">+0x00</span>
                        <span class="field-name">id:</span>
                        <span class="field-val highlight-id">${c.id}</span>
                    </div>
                    <div class="payload-field">
                        <span class="field-offset">+0x04</span>
                        <span class="field-name">name:</span>
                        <span class="field-val highlight-name">${escapeHtml(cleanName)}</span>
                    </div>
                    <div class="payload-field">
                        <span class="field-offset">+0x36</span>
                        <span class="field-name">phone:</span>
                        <span class="field-val highlight-phone">${escapeHtml(c.phone)}</span>
                    </div>
                    <div class="payload-field">
                        <span class="field-offset">+0x4A</span>
                        <span class="field-name">email:</span>
                        <span class="field-val highlight-email">${c.email ? escapeHtml(c.email) : '// UNSET'}</span>
                    </div>
                </div>

                <div class="node-block-pointer">
                    <div class="pointer-meta">
                        <span class="field-offset">+0x88</span>
                        <span class="field-name">*next:</span>
                    </div>
                    <div class="pointer-dest ${isNullNext ? 'is-null' : ''}">
                        ${nextAddr} ${isNullNext ? '(NULL)' : ''}
                    </div>
                </div>
            </div>
        `;

        if (idx < allContacts.length - 1) {
            html += `
                <div class="pointer-arrow-connector">
                    <div class="arrow-line"></div>
                    <div class="arrow-head">►</div>
                </div>
            `;
        }
    });

    html += `
        <div class="pointer-arrow-connector">
            <div class="arrow-line"></div>
            <div class="arrow-head">►</div>
        </div>
        <div class="memory-register null-block">
            <div class="reg-header">TERMINATOR</div>
            <div class="reg-body">
                <div class="reg-ptr-val">0x00000000</div>
                <div class="reg-label">&gt; NULL_BOUNDARY</div>
            </div>
        </div>
    `;

    chain.innerHTML = html;
}

// Retro Terminal Toast Notification Manager
function showToast(message, type = 'info') {
    const container = document.getElementById('toast-container');
    const toast = document.createElement('div');
    toast.className = `terminal-toast ${type}`;

    let prefix = '[SYS_INFO]';
    if (type === 'success') prefix = '[SYS_OK]';
    if (type === 'error') prefix = '[SYS_ERR]';

    toast.innerHTML = `
        <span class="toast-prefix">${prefix}</span>
        <span class="toast-msg">${escapeHtml(message)}</span>
    `;
    container.appendChild(toast);

    setTimeout(() => {
        toast.style.opacity = '0';
        toast.style.transform = 'translateY(10px) scale(0.96)';
        toast.style.transition = 'all 0.2s cubic-bezier(0.16, 1, 0.3, 1)';
        setTimeout(() => toast.remove(), 200);
    }, 3400);
}

// HTML XSS Escaping Helper
function escapeHtml(str) {
    if (!str) return '';
    return String(str)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;')
        .replace(/'/g, '&#39;');
}

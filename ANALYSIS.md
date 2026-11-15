# Engineering Analysis — Baseline Buffer vs. Gap Buffer

## 1. Executive Summary
While the `BaselineBuffer` (`std::string`) provides a simple, bug-free reference implementation, it suffers from $O(N)$ memory shifting on every localized edit. The **Gap Buffer** resolves this bottleneck by maintaining an empty gap at the active cursor position, turning sequential edits into $O(1)$ operations while preserving contiguous memory cache locality.

---

## 2. Architectural Comparison Matrix

Let $N$ be the active text length, $K$ be the length of inserted/deleted text, $C$ be the total allocated capacity, and $\Delta = |\text{targetIndex} - \text{gapStart}|$ be the cursor jump distance.

| Metric | Baseline Buffer (`std::string`) | Gap Buffer (`std::vector<char>`) | Engineering Winner |
| :--- | :--- | :--- | :--- |
| **Insert Complexity** | $O(N + K)$ worst-case; $O(K)$ amortized at end | $O(\Delta + K)$ worst-case; **$O(K)$ amortized at cursor** | **Gap Buffer** |
| **Delete Complexity** | $O(N)$ worst-case; $O(1)$ at end | $O(\Delta)$ worst-case; **$O(1)$ at cursor** | **Gap Buffer** |
| **Cursor Movement** | $O(1)$ (stateless index) | $O(\Delta)$ (must shift characters across the gap) | **Baseline** |
| **Gap Expansion** | Geometric growth ($1.5\times$ or $2\times$), $O(N)$ copy | Geometric growth ($2\times$), $O(N)$ split copy | **Tie** |
| **Memory Overhead** | Minimal (only `std::string` capacity slack) | $O(C - N)$ unused gap bytes + 2 `size_t` indices | **Baseline** |
| **Worst-case Performance** | $O(N)$ on every edit near index `0` | $O(N)$ when jumping across file or expanding gap | **Gap Buffer** |
| **Typical Editing Workload** | Poor on large files due to repeated shifting | **Optimal ($O(1)$)** due to spatial locality | **Gap Buffer** |

---

## 3. Deep-Dive Analysis

### 1. Insert Complexity
* **Baseline:** Inserting $K$ bytes at index $i$ requires shifting $N - i$ bytes to the right, costing $O(N + K)$.
* **Gap Buffer:** Once the gap is positioned at the cursor, insertion simply writes $K$ bytes into the pre-allocated gap and increments `gapStart_` in **$O(K)$** time (assuming no expansion is needed).

### 2. Delete Complexity
* **Baseline:** Deleting $K$ bytes at index $i$ shifts $N - (i + K)$ trailing bytes left, costing $O(N)$.
* **Gap Buffer:** Deletion at the cursor requires **zero memory movement**. Incrementing `gapEnd_` (or decrementing `gapStart_` for backspace) absorbs the deleted bytes into the gap in **$O(1)$** time.

### 3. Cursor Movement
* **Baseline:** Does not maintain cursor state; every operation receives an explicit index at $O(1)$ lookup cost.
* **Gap Buffer:** Moving the cursor to a new position requires physically copying $\Delta = |\text{target} - \text{gapStart}|$ bytes from one side of the gap to the other, costing **$O(\Delta)$**.

### 4. Gap Expansion
* Both structures use **geometric capacity growth** (doubling capacity when full) to achieve **$O(1)$ amortized** allocation cost.
* When the gap fills up (`GapSize() < K`), Gap Buffer allocates a larger contiguous block, copies the prefix `[0, gapStart_)` to the start, and copies the suffix `[gapEnd_, capacity)` to the end in $O(N)$ time.

### 5. Memory Overhead
* **Baseline:** Consumes $O(N)$ memory with minimal unused capacity managed automatically by `std::string`.
* **Gap Buffer:** Explicitly trades space for speed. It maintains an intentional empty region (`gapEnd_ - gapStart_`) plus two metadata pointers (`gapStart_`, `gapEnd_`). Deleted bytes remain in memory as stale data until overwritten or reallocated.

### 6. Worst-case Performance
* **Baseline:** Degrades consistently to $O(N)$ whenever editing near the beginning of a large document, even if edits are sequential.
* **Gap Buffer:** Degrades to $O(N)$ only during **non-local random jumps** (e.g., alternating edits between index `0` and index `N`) or during periodic **gap reallocations**.

### 7. Typical Editing Workload (Spatial Locality)
* Real-world human typing exhibits high **spatial locality**: users type, backspace, and edit dozens of consecutive characters at a single cursor location before moving elsewhere.
* **Result:** In typical editing workloads, $\Delta \approx 0$ or $\Delta = 1$ for 99% of keystrokes. Thus, Gap Buffer delivers near-constant **$O(1)$** latency with exceptional **CPU cache locality**, outperforming `BaselineBuffer` by orders of magnitude on large files.

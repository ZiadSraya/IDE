# Text Buffer Engine — Gap Buffer Design Document (v2.0)

## 1. Architecture & Memory Layout
The `GapBuffer` implements `ITextBuffer` using a single contiguous dynamic array (`std::vector<char>`) partitioned into three logical regions:
1. **Prefix Segment `[0, gapStart_)`:** Active UTF-8 bytes before the cursor.
2. **The Gap `[gapStart_, gapEnd_)`:** Pre-allocated unused buffer space positioned at the active cursor.
3. **Suffix Segment `[gapEnd_, capacity)`:** Active UTF-8 bytes after the cursor.

```text
Physical Array: [ P | r | e | _ | _ | _ | _ | S | u | f ]
Indices:          0   1   2   3           7   8   9   10
                              ^           ^
                          gapStart_    gapEnd_
```

---

## 2. Coordinate System (Logical vs. Physical Offsets)
* **Logical Byte Offset:** The zero-based index exposed via `ITextBuffer` API (`0 <= index <= Length()`). It excludes the gap entirely.
* **Physical Array Index:** The actual index inside `buffer_` (`0 <= phys <= buffer_.size()`).
* **Translation Formula:**
  * If `logical < gapStart_`: `physical = logical`
  * If `logical >= gapStart_`: `physical = logical + (gapEnd_ - gapStart_)`

---

## 3. Core Algorithms & State Transitions
* **Gap Movement (`MoveGap(target)`):** Shifts `|target - gapStart_|` bytes across the gap to relocate the empty region to `target`. Cost: $O(|\text{target} - \text{gapStart\_}|)$.
* **Gap Expansion (`ExpandGap(required)`):** Triggered when `gapEnd_ - gapStart_ < required`. Uses a geometric growth policy (`additionalSize = max(required, max(16, buffer_.size()))`), resizes the underlying `std::vector<char>` in-place via `buffer_.resize()`, and shifts the suffix segment `[gapEnd_, oldSize)` right to the end of the resized buffer. Cost: $O(N)$ worst-case, $O(1)$ amortized.
* **Insertion (`Insert(index, text)`):** Moves gap to `index`, ensures capacity, writes `text` into `[gapStart_, gapStart_ + text.size())`, and advances `gapStart_`.
* **Deletion (`Delete(index, length)`):** Moves gap to `index` and increments `gapEnd_ += length`, absorbing deleted bytes into the gap without zeroing memory.

---

## 4. Boundary, Empty & Unicode Policies
* **Out-of-Bounds:** Throws `std::out_of_range` if `index > Length()` or `length > Length() - index`.
* **Empty Operations:** `Insert(0, "")` and `Delete(0, 0)` execute as $O(1)$ no-ops.
* **Unicode Strategy:** Operates strictly on raw **UTF-8 byte offsets** (`char`). Multi-byte character boundary validation is deferred to upper editor layers.

---

## 5. System Invariants
1. **Pointer Ordering:** `0 <= gapStart_ <= gapEnd_ <= buffer_.size()` holds before and after every public operation.
2. **Length Synchronization:** `Length() == buffer_.size() - (gapEnd_ - gapStart_)` is strictly preserved.
3. **Gap Isolation:** Read operations (`GetText`, `GetRange`) never expose stale/garbage bytes residing inside `[gapStart_, gapEnd_)`.
4. **Strict Encapsulation:** `buffer_`, `gapStart_`, and `gapEnd_` are `private` and mutated solely through validated internal helpers.

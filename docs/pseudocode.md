# Pseudocode

Notation: `T.root` is the root, `x.key` is the IATA code of node `x`, `NIL` is an empty link.
All loops over the tree are written with `while` (or an explicit stack/queue), so a 6,071-deep
chain cannot overflow the call stack. Only the balanced build is recursive, and its depth is ⌈log₂ n⌉.

## Load and build (Task 1)

```
LOAD(file)
    records ← empty array; skipped ← 0
    for each line in file
        fields ← SPLIT-CSV(line)            // commas inside "..." are not separators, "" → "
        if fields[5] = "\N" then skipped ← skipped + 1; continue
        append Airport(fields[2], fields[3], fields[4], fields[5], fields[6], fields[7], fields[8]) to records
    BUILD(T, records)
    report |records|, skipped, height, average depth

BUILD(T, records)
    T.root ← NIL; duplicates ← 0
    for i ← 0 to n − 1
        if INSERT(T, records[i]) = false then duplicates ← duplicates + 1; report records[i].key
    return duplicates
```

## Insert

```
INSERT(T, a)
    if T.root = NIL then T.root ← NEW-NODE(a); return true          // I1
    x ← T.root
    while true
        if a.key < x.key                                             // I2
            if x.left = NIL then x.left ← NEW-NODE(a); return true
            x ← x.left
        else if a.key > x.key                                        // I3
            if x.right = NIL then x.right ← NEW-NODE(a); return true
            x ← x.right
        else return false                                            // I4 duplicate
```

## Search

```
SEARCH(T, k)
    x ← T.root; comparisons ← 0; path ← empty
    while x ≠ NIL
        comparisons ← comparisons + 1; append x.key to path
        if k = x.key then return x, path, comparisons
        if k < x.key then x ← x.left else x ← x.right
    return NIL, path, comparisons

MINIMUM(x)   while x.left ≠ NIL:  x ← x.left;  return x
MAXIMUM(x)   while x.right ≠ NIL: x ← x.right; return x
```

## Delete

```
DELETE(T, k)
    parent ← NIL; x ← T.root
    while x ≠ NIL and x.key ≠ k
        parent ← x
        if k < x.key then x ← x.left else x ← x.right
    if x = NIL then return NOT-FOUND

    if x.left ≠ NIL and x.right ≠ NIL                                // D3 two children
        sp ← x; s ← x.right
        while s.left ≠ NIL: sp ← s; s ← s.left                       // inorder successor
        x.record ← s.record
        if sp = x then sp.right ← s.right else sp.left ← s.right     // s has no left child
        free s
    else                                                             // D1 leaf / D2 one child
        child ← (x.left ≠ NIL) ? x.left : x.right                    // NIL for a leaf
        if parent = NIL then T.root ← child
        else if parent.left = x then parent.left ← child
        else parent.right ← child
        free x
```

## Height and average depth

```
HEIGHT(T)                                     // level-order, counts levels
    if T.root = NIL return −1
    Q ← queue(T.root); levels ← 0
    while Q not empty
        w ← |Q|
        for i ← 0 to w − 1
            x ← DEQUEUE(Q); enqueue x.left, x.right if not NIL
        levels ← levels + 1
    return levels − 1

AVERAGE-DEPTH(T)
    Q ← queue((T.root, 0)); total ← 0; n ← 0
    while Q not empty
        (x, d) ← DEQUEUE(Q); total ← total + d; n ← n + 1
        enqueue (x.left, d + 1), (x.right, d + 1) if not NIL
    return total / n
```

## Traversals (Task 4)

```
INORDER(T)                                    // alphabetical listing
    S ← empty stack; x ← T.root
    while x ≠ NIL or S not empty
        while x ≠ NIL: PUSH(S, x); x ← x.left
        x ← POP(S); output x; x ← x.right

PREORDER(T)                                   // used by save
    S ← stack(T.root)
    while S not empty
        x ← POP(S); output x
        PUSH(S, x.right) if not NIL; PUSH(S, x.left) if not NIL

SAVE(T, file)   write PREORDER(T), one record per line
LOAD-PREORDER(file)   T.root ← NIL; for each record in file order: INSERT(T, record)
    // the first record is the old root, and every later record follows the same
    // comparisons as before, so it lands in the same position → identical shape
```

## Validate

```
VALIDATE(T)
    S ← stack((T.root, −∞, +∞)); seen ← 0
    while S not empty
        (x, lo, hi) ← POP(S)
        if not (lo < x.key < hi) return false
        seen ← seen + 1
        PUSH(S, (x.left, lo, x.key))  if x.left ≠ NIL
        PUSH(S, (x.right, x.key, hi)) if x.right ≠ NIL
    return seen = T.count
```

## Range query (Task 5)

```
RANGE(T, from, to)
    S ← empty stack; x ← T.root; visited ← 0; result ← empty
    while x ≠ NIL or S not empty
        while x ≠ NIL
            visited ← visited + 1; PUSH(S, x)
            if from < x.key then x ← x.left else x ← NIL     // left subtree all < from: skip
        y ← POP(S)
        if from ≤ y.key ≤ to then append y to result
        if y.key < to then x ← y.right else x ← NIL          // right subtree all > to: skip
    return result, visited
```

## Balanced build (Task 6, `rebalance`)

```
REBALANCE(T)
    A ← INORDER(T)                  // sorted array
    T.root ← BUILD-BALANCED(A, 0, |A| − 1)

BUILD-BALANCED(A, lo, hi)
    if lo > hi return NIL
    mid ← lo + ⌊(hi − lo) / 2⌋
    x ← NEW-NODE(A[mid])
    x.left  ← BUILD-BALANCED(A, lo, mid − 1)
    x.right ← BUILD-BALANCED(A, mid + 1, hi)
    return x
```

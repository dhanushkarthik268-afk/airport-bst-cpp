// AirportBST.h - binary search tree of airports keyed by IATA code.
// insert, search, remove, height, validate and the traversals are written iteratively,
// so a 6,071-deep chain (sorted insertion order) cannot overflow the call stack.
#ifndef AIRPORT_BST_H
#define AIRPORT_BST_H

#include "Airport.h"

#include <string>
#include <vector>

class AirportBST {
public:
    // Which deletion case remove() used.
    enum class DeleteCase { NotFound, D1_Leaf, D2_LeftOnly, D2_RightOnly, D3_TwoChildren };
    // Node shapes, used by the tests to pick one code for each deletion case.
    enum class Shape { Leaf, LeftOnly, RightOnly, TwoChildren };

    AirportBST();
    ~AirportBST();
    AirportBST(const AirportBST&) = delete;
    AirportBST& operator=(const AirportBST&) = delete;

    void clear();
    int  size() const { return count; }
    bool empty() const { return root == nullptr; }

    // Insert. Returns false (and changes nothing) if the code is already present (rule I4).
    bool insert(const Airport& airport);

    // Search. Optionally records every code compared and the number of comparisons.
    const Airport* find(const std::string& code,
                        std::vector<std::string>* path = nullptr,
                        int* comparisons = nullptr) const;
    bool contains(const std::string& code) const;

    // Delete. `detail` receives a human-readable description of the case used.
    DeleteCase remove(const std::string& code, std::string* detail = nullptr);

    const Airport* minimum() const;
    const Airport* maximum() const;

    int    height() const;          // edges on the longest root-to-leaf path; -1 if empty
    double averageDepth() const;    // mean depth of a node (the root has depth 0)

    std::vector<Airport> inorder() const;                        // alphabetical order
    std::vector<Airport> preorder() const;                       // used by save
    std::vector<std::vector<std::string>> levelOrder(int maxLevels = -1) const;

    // Checks lo < key < hi at every node, tightening (lo, hi) on the way down.
    bool validate() const;

    // Appends every airport with FROM <= code <= TO to `out` (in order) and returns the
    // number of nodes looked at. Subtrees that lie outside the range are never entered.
    int range(const std::string& from, const std::string& to, std::vector<Airport>& out) const;

    // Replace the tree with a balanced one built from sorted records (middle rule).
    void buildBalanced(const std::vector<Airport>& sorted);
    void rebalance();

    // Helpers for the tests.
    std::string rootCode() const;
    std::string deepestCode() const;
    // First node in level order (root excluded) with the given shape; "" if none.
    std::string findCodeWithShape(Shape shape) const;

private:
    struct Node {
        Airport data;
        Node* left;
        Node* right;
        explicit Node(const Airport& a) : data(a), left(nullptr), right(nullptr) {}
    };

    Node* root;     // private, as required
    int count;

    Node* buildBalancedRange(const std::vector<Airport>& sorted, int lo, int hi);
};

#endif

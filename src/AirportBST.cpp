// AirportBST.cpp - binary search tree operations.
#include "AirportBST.h"

#include <queue>
#include <stack>
#include <utility>

AirportBST::AirportBST() : root(nullptr), count(0) {}

AirportBST::~AirportBST() { clear(); }

// Free every node with an explicit stack (no recursion, safe for a chain).
void AirportBST::clear() {
    std::stack<Node*> st;
    if (root) st.push(root);
    while (!st.empty()) {
        Node* node = st.top();
        st.pop();
        if (node->left)  st.push(node->left);
        if (node->right) st.push(node->right);
        delete node;
    }
    root = nullptr;
    count = 0;
}

// ---------------------------------------------------------------- insert
// I1 empty tree -> new node becomes the root
// I2 key < node -> go left;  I3 key > node -> go right (attach at the first null link)
// I4 key == node -> duplicate, reject
bool AirportBST::insert(const Airport& airport) {
    if (root == nullptr) {                         // I1
        root = new Node(airport);
        count++;
        return true;
    }
    Node* cur = root;
    while (true) {
        if (airport.code < cur->data.code) {       // I2
            if (cur->left == nullptr) { cur->left = new Node(airport); break; }
            cur = cur->left;
        } else if (airport.code > cur->data.code) { // I3
            if (cur->right == nullptr) { cur->right = new Node(airport); break; }
            cur = cur->right;
        } else {                                   // I4
            return false;
        }
    }
    count++;
    return true;
}

// ---------------------------------------------------------------- search
const Airport* AirportBST::find(const std::string& code, std::vector<std::string>* path,
                                int* comparisons) const {
    int compared = 0;
    Node* cur = root;
    while (cur != nullptr) {
        compared++;
        if (path) path->push_back(cur->data.code);
        if (code == cur->data.code) {
            if (comparisons) *comparisons = compared;
            return &cur->data;
        }
        cur = (code < cur->data.code) ? cur->left : cur->right;
    }
    if (comparisons) *comparisons = compared;
    return nullptr;
}

bool AirportBST::contains(const std::string& code) const {
    Node* cur = root;
    while (cur != nullptr) {
        if (code == cur->data.code) return true;
        cur = (code < cur->data.code) ? cur->left : cur->right;
    }
    return false;
}

const Airport* AirportBST::minimum() const {
    if (!root) return nullptr;
    Node* cur = root;
    while (cur->left) cur = cur->left;
    return &cur->data;
}

const Airport* AirportBST::maximum() const {
    if (!root) return nullptr;
    Node* cur = root;
    while (cur->right) cur = cur->right;
    return &cur->data;
}

// ---------------------------------------------------------------- delete
// D1 leaf                -> unlink it
// D2 one child           -> the child takes the node's place
// D3 two children        -> copy the inorder successor (min of right subtree) into the
//                           node, then delete the successor (it has no left child)
AirportBST::DeleteCase AirportBST::remove(const std::string& code, std::string* detail) {
    Node* parent = nullptr;
    Node* cur = root;
    while (cur != nullptr && cur->data.code != code) {
        parent = cur;
        cur = (code < cur->data.code) ? cur->left : cur->right;
    }
    if (cur == nullptr) {
        if (detail) *detail = "not found";
        return DeleteCase::NotFound;
    }

    DeleteCase result;
    if (cur->left != nullptr && cur->right != nullptr) {
        // D3: find the successor and its parent
        Node* succParent = cur;
        Node* succ = cur->right;
        while (succ->left != nullptr) {
            succParent = succ;
            succ = succ->left;
        }
        if (detail) *detail = "case D3: two children, replaced by inorder successor " + succ->data.code;
        cur->data = succ->data;
        // the successor has at most a right child
        if (succParent == cur) succParent->right = succ->right;
        else                   succParent->left  = succ->right;
        delete succ;
        result = DeleteCase::D3_TwoChildren;
    } else {
        Node* child = (cur->left != nullptr) ? cur->left : cur->right;
        if (child == nullptr) {
            result = DeleteCase::D1_Leaf;
            if (detail) *detail = "case D1: leaf";
        } else if (cur->left != nullptr) {
            result = DeleteCase::D2_LeftOnly;
            if (detail) *detail = "case D2: only a left child, " + child->data.code;
        } else {
            result = DeleteCase::D2_RightOnly;
            if (detail) *detail = "case D2: only a right child, " + child->data.code;
        }
        if (parent == nullptr)          root = child;
        else if (parent->left == cur)   parent->left = child;
        else                            parent->right = child;
        delete cur;
    }
    count--;
    return result;
}

// ---------------------------------------------------------------- height / depth
int AirportBST::height() const {
    if (!root) return -1;
    int levels = 0;
    std::queue<Node*> q;
    q.push(root);
    while (!q.empty()) {
        int width = static_cast<int>(q.size());
        for (int i = 0; i < width; i++) {
            Node* node = q.front();
            q.pop();
            if (node->left)  q.push(node->left);
            if (node->right) q.push(node->right);
        }
        levels++;
    }
    return levels - 1;
}

double AirportBST::averageDepth() const {
    if (!root) return 0.0;
    long long totalDepth = 0;
    long long nodes = 0;
    std::queue<std::pair<Node*, int>> q;
    q.push({root, 0});
    while (!q.empty()) {
        Node* node = q.front().first;
        int depth = q.front().second;
        q.pop();
        totalDepth += depth;
        nodes++;
        if (node->left)  q.push({node->left, depth + 1});
        if (node->right) q.push({node->right, depth + 1});
    }
    return static_cast<double>(totalDepth) / static_cast<double>(nodes);
}

// ---------------------------------------------------------------- traversals
std::vector<Airport> AirportBST::inorder() const {
    std::vector<Airport> out;
    out.reserve(count);
    std::stack<Node*> st;
    Node* cur = root;
    while (cur != nullptr || !st.empty()) {
        while (cur != nullptr) { st.push(cur); cur = cur->left; }
        cur = st.top();
        st.pop();
        out.push_back(cur->data);
        cur = cur->right;
    }
    return out;
}

std::vector<Airport> AirportBST::preorder() const {
    std::vector<Airport> out;
    out.reserve(count);
    std::stack<Node*> st;
    if (root) st.push(root);
    while (!st.empty()) {
        Node* node = st.top();
        st.pop();
        out.push_back(node->data);
        if (node->right) st.push(node->right);   // pushed first so left is visited first
        if (node->left)  st.push(node->left);
    }
    return out;
}

std::vector<std::vector<std::string>> AirportBST::levelOrder(int maxLevels) const {
    std::vector<std::vector<std::string>> levels;
    if (!root) return levels;
    std::queue<Node*> q;
    q.push(root);
    while (!q.empty() && (maxLevels < 0 || static_cast<int>(levels.size()) < maxLevels)) {
        int width = static_cast<int>(q.size());
        std::vector<std::string> level;
        for (int i = 0; i < width; i++) {
            Node* node = q.front();
            q.pop();
            level.push_back(node->data.code);
            if (node->left)  q.push(node->left);
            if (node->right) q.push(node->right);
        }
        levels.push_back(level);
    }
    return levels;
}

// ---------------------------------------------------------------- validate
// Every node must satisfy lo < key < hi. Going left, hi becomes the key;
// going right, lo becomes the key. nullptr means "no bound".
bool AirportBST::validate() const {
    struct Frame { Node* node; const std::string* lo; const std::string* hi; };
    std::stack<Frame> st;
    if (root) st.push({root, nullptr, nullptr});
    int seen = 0;
    while (!st.empty()) {
        Frame f = st.top();
        st.pop();
        const std::string& key = f.node->data.code;
        if (f.lo && !(*f.lo < key)) return false;
        if (f.hi && !(key < *f.hi)) return false;
        seen++;
        if (f.node->left)  st.push({f.node->left, f.lo, &key});
        if (f.node->right) st.push({f.node->right, &key, f.hi});
    }
    return seen == count;   // also catches a broken node count
}

// ---------------------------------------------------------------- range
// Inorder walk that only goes left when key > FROM and only goes right when key < TO.
int AirportBST::range(const std::string& from, const std::string& to,
                      std::vector<Airport>& out) const {
    int visited = 0;
    std::stack<Node*> st;
    Node* cur = root;
    while (cur != nullptr || !st.empty()) {
        while (cur != nullptr) {
            visited++;
            st.push(cur);
            cur = (from < cur->data.code) ? cur->left : nullptr;   // prune left subtree
        }
        Node* node = st.top();
        st.pop();
        const std::string& key = node->data.code;
        if (from <= key && key <= to) out.push_back(node->data);
        cur = (key < to) ? node->right : nullptr;                  // prune right subtree
    }
    return visited;
}

// ---------------------------------------------------------------- balanced build
AirportBST::Node* AirportBST::buildBalancedRange(const std::vector<Airport>& sorted,
                                                 int lo, int hi) {
    if (lo > hi) return nullptr;
    int mid = lo + (hi - lo) / 2;                 // middle element becomes the root
    Node* node = new Node(sorted[mid]);
    count++;
    node->left  = buildBalancedRange(sorted, lo, mid - 1);
    node->right = buildBalancedRange(sorted, mid + 1, hi);
    return node;                                   // recursion depth is only ~log2(n)
}

void AirportBST::buildBalanced(const std::vector<Airport>& sorted) {
    clear();
    root = buildBalancedRange(sorted, 0, static_cast<int>(sorted.size()) - 1);
}

void AirportBST::rebalance() {
    std::vector<Airport> sorted = inorder();
    buildBalanced(sorted);
}

// ---------------------------------------------------------------- test helpers
std::string AirportBST::rootCode() const {
    return root ? root->data.code : "";
}

std::string AirportBST::deepestCode() const {
    if (!root) return "";
    std::queue<Node*> q;
    q.push(root);
    Node* last = root;
    while (!q.empty()) {
        last = q.front();
        q.pop();
        if (last->left)  q.push(last->left);
        if (last->right) q.push(last->right);
    }
    return last->data.code;   // the last node in level order is on the deepest level
}

std::string AirportBST::findCodeWithShape(Shape shape) const {
    if (!root) return "";
    std::queue<Node*> q;
    q.push(root);
    while (!q.empty()) {
        Node* node = q.front();
        q.pop();
        bool hasL = node->left != nullptr;
        bool hasR = node->right != nullptr;
        bool match = (shape == Shape::Leaf        && !hasL && !hasR) ||
                     (shape == Shape::LeftOnly    &&  hasL && !hasR) ||
                     (shape == Shape::RightOnly   && !hasL &&  hasR) ||
                     (shape == Shape::TwoChildren &&  hasL &&  hasR);
        if (match && node != root) return node->data.code;
        if (hasL) q.push(node->left);
        if (hasR) q.push(node->right);
    }
    return "";
}

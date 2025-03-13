

#include <iostream>
using namespace std;

class BTreeNode {
  int *keys;
  int t;     
  BTreeNode **C;
  int n;
  bool leaf;

   public:
  BTreeNode(int _t, bool _leaf);

  void traverse();

  int findKey(int k);
  void insertNonFull(int k);
  void splitChild(int i, BTreeNode *y);
  void deletion(int k);
  void removeFromLeaf(int idx);
  void removeFromNonLeaf(int idx);
  int getPredecessor(int idx);
  int getSuccessor(int idx);
  void fill(int idx);
  void borrowFromPrev(int idx);
  void borrowFromNext(int idx);
  void merge(int idx);
  BTreeNode* search(int s_key);
  friend class BTree;
};

class BTree {
  BTreeNode *root;
  int t;

   public:
  BTree(int _t) {
    root = NULL;
    t = _t;
  }

  void traversal() {
    if (root != NULL)
      root->traverse();
  }

  void insertion(int k);

  void deletion(int k);

  BTreeNode* search(int s_key) {
        return root ? root->search(s_key) : nullptr;  // Start the search from the root
    }
};

// B tree node
BTreeNode::BTreeNode(int t1, bool leaf1) {
  t = t1;
  leaf = leaf1;

  keys = new int[2 * t - 1];
  C = new BTreeNode *[2 * t];

  n = 0;
}

// Find the key
int BTreeNode::findKey(int k) {
  int idx = 0;
  while (idx < n && keys[idx] < k)
    ++idx;
  return idx;
}

// Deletion operation
void BTreeNode::deletion(int k) {
  int idx = findKey(k);

  if (idx < n && keys[idx] == k) {
    if (leaf)
      removeFromLeaf(idx);
    else
      removeFromNonLeaf(idx);
  } else {
    if (leaf) {
      cout << "The key " << k << " is does not exist in the tree\n";
      return;
    }

    bool flag = ((idx == n) ? true : false);

    if (C[idx]->n < t)
      fill(idx);

    if (flag && idx > n)
      C[idx - 1]->deletion(k);
    else
      C[idx]->deletion(k);
  }
  return;
}

// Remove from the leaf
void BTreeNode::removeFromLeaf(int idx) {
  for (int i = idx + 1; i < n; ++i)
    keys[i - 1] = keys[i];

  n--;

  return;
}

// Delete from non leaf node
void BTreeNode::removeFromNonLeaf(int idx) {
  int k = keys[idx];

  if (C[idx]->n >= t) {
    int pred = getPredecessor(idx);
    keys[idx] = pred;
    C[idx]->deletion(pred);
  }

  else if (C[idx + 1]->n >= t) {
    int succ = getSuccessor(idx);
    keys[idx] = succ;
    C[idx + 1]->deletion(succ);
  }

  else {
    merge(idx);
    C[idx]->deletion(k);
  }
  return;
}

int BTreeNode::getPredecessor(int idx) {
  BTreeNode *cur = C[idx];
  while (!cur->leaf)
    cur = cur->C[cur->n];

  return cur->keys[cur->n - 1];
}

int BTreeNode::getSuccessor(int idx) {
  BTreeNode *cur = C[idx + 1];
  while (!cur->leaf)
    cur = cur->C[0];

  return cur->keys[0];
}

void BTreeNode::fill(int idx) {
  if (idx != 0 && C[idx - 1]->n >= t)
    borrowFromPrev(idx);

  else if (idx != n && C[idx + 1]->n >= t)
    borrowFromNext(idx);

  else {
    if (idx != n)
      merge(idx);
    else
      merge(idx - 1);
  }
  return;
}
BTreeNode* BTreeNode::search(int s_key) {
    int i = 0;
    while(i < n && s_key > keys[i]) {
        i++;
    } //Aise loop dikhe to ye max time desired child tak ya key tak pahuchn ke liye hai

    // If key is found in this node, return the current node
    if(i < n && keys[i] == s_key) {
        return this;
    }

    // If it's a leaf node and key is not found, return nullptr
    if(leaf) {
        return nullptr;
    }

    // Recursively search in the appropriate child node
    return C[i]->search(s_key);  // Correct child node to search
}
// Borrow from previous
void BTreeNode::borrowFromPrev(int idx) {
  BTreeNode *child = C[idx];
  BTreeNode *sibling = C[idx - 1];

  for (int i = child->n - 1; i >= 0; --i)
    child->keys[i + 1] = child->keys[i];//child me keys shift krke space bana rhe hai

  if (!child->leaf) {
    for (int i = child->n; i >= 0; --i) //Child Pointers ko bhi shift kar rhe if node is non-leaf
      child->C[i + 1] = child->C[i];
  }

  child->keys[0] = keys[idx - 1]; //child me parent ki key put krdi gayi (Rotation ho rha hai)

  if (!child->leaf)
    child->C[0] = sibling->C[sibling->n]; //Agar child non-leaf hai, to sibling ka last child pointer ko child ke first child pointer ke slot me insert karte hain

  keys[idx - 1] = sibling->keys[sibling->n - 1];//Rotation of Keys)//sibling ki last key ko parent key ke slot (keys[idx - 1]) me transfer karte hain, taaki parent properly updated rahe.

  child->n += 1;
  sibling->n -= 1;

  return;
}

// Borrow from the next
void BTreeNode::borrowFromNext(int idx) {
  BTreeNode *child = C[idx];
  BTreeNode *sibling = C[idx + 1];

  child->keys[(child->n)] = keys[idx];

  if (!(child->leaf))
    child->C[(child->n) + 1] = sibling->C[0];

  keys[idx] = sibling->keys[0];

  for (int i = 1; i < sibling->n; ++i)
    sibling->keys[i - 1] = sibling->keys[i];

  if (!sibling->leaf) {
    for (int i = 1; i <= sibling->n; ++i)
      sibling->C[i - 1] = sibling->C[i];
  }

  child->n += 1;
  sibling->n -= 1;

  return;
}

// Merge
void BTreeNode::merge(int idx) {
  BTreeNode *child = C[idx];
  BTreeNode *sibling = C[idx + 1];

  child->keys[t - 1] = keys[idx];//Parent ki key child me daali ja rhi hai

  for (int i = 0; i < sibling->n; ++i)
    child->keys[i + t] = sibling->keys[i];//All keys of the sibling are appended(moved) to the child.

  if (!child->leaf) { //Agar leaf node nhi hai to saare sibling ke saare child pointer bhi copy ho rhe hai
    for (int i = 0; i <= sibling->n; ++i)
      child->C[i + t] = sibling->C[i];
  }

  for (int i = idx + 1; i < n; ++i)
    keys[i - 1] = keys[i];

  for (int i = idx + 2; i <= n; ++i)
    C[i - 1] = C[i];

  child->n += sibling->n + 1;
  n--;

  delete (sibling);
  return;
}

// Insertion operation
void BTree::insertion(int k) {
  if (root == NULL) {
    root = new BTreeNode(t, true);
    root->keys[0] = k;
    root->n = 1;
  } else {
    if (root->n == 2 * t - 1) {//if root is full 
      BTreeNode *s = new BTreeNode(t, false);

      s->C[0] = root;

      s->splitChild(0, root);

      int i = 0;
      if (s->keys[0] < k)
        i++;
      s->C[i]->insertNonFull(k);

      root = s;
    } else
      root->insertNonFull(k);
  }
}

// Insertion non full
void BTreeNode::insertNonFull(int k) {
  int i = n - 1;

  if (leaf == true) {
    while (i >= 0 && keys[i] > k) {
      keys[i + 1] = keys[i];
      i--;
    }

    keys[i + 1] = k;
    n = n + 1;
  } else {
    while (i >= 0 && keys[i] > k)
      i--;

    if (C[i + 1]->n == 2 * t - 1) {
      splitChild(i + 1, C[i + 1]);

      if (keys[i + 1] < k)
        i++;
    }
    C[i + 1]->insertNonFull(k);
  }
}

// Split child
void BTreeNode::splitChild(int i, BTreeNode *y) {
  BTreeNode *z = new BTreeNode(y->t, y->leaf);
  z->n = t - 1;

  for (int j = 0; j < t - 1; j++)
    z->keys[j] = y->keys[j + t];

  if (y->leaf == false) {
    for (int j = 0; j < t; j++)
      z->C[j] = y->C[j + t];
  }

  y->n = t - 1;

  for (int j = n; j >= i + 1; j--)
    C[j + 1] = C[j];

  C[i + 1] = z;

  for (int j = n - 1; j >= i; j--)
    keys[j + 1] = keys[j];

  keys[i] = y->keys[t - 1];

  n = n + 1;
}

// Traverse
void BTreeNode::traverse() {
  int i;
  for (i = 0; i < n; i++) {
    if (leaf == false)
      C[i]->traverse();
    cout << " " << keys[i];
  }

  if (leaf == false)
    C[i]->traverse();
}

// Delete Operation
void BTree::deletion(int k) {
  if (!root) {
    cout << "The tree is empty\n";
    return;
  }

  root->deletion(k); //Node wala deletion function call hoga

  if (root->n == 0) {//if root me keys 0 ho jati hai to
    BTreeNode *tmp = root; //tmep node pointer hai jo root ko point kar rha hai
    if (root->leaf) //Agar root leaaf hai(yani no child) to wo NULL ko point karega
      root = NULL;
    else          //Root leaf nhi hai (mtlb uske child hai)
      root = root->C[0]; //to root ko uske first child ki taraf reference kar diya jaega

    delete tmp;
  }
  return;
}

int main() {
int choice, key;
    BTree t(3);
     cout << "\n====== B-Tree Operations Menu ======" << endl;
        cout << "1. Insert a key" << endl;
        cout << "2. Search for a key" << endl;
        cout << "3. Delete a key" << endl;
        cout << "4. Display the tree" << endl;
        cout << "5. Exit" << endl;

    while (true) {
        // cout << "\n====== B-Tree Operations Menu ======" << endl;
        // cout << "1. Insert a key" << endl;
        // cout << "2. Search for a key" << endl;
        // cout << "3. Delete a key" << endl;
        // cout << "4. Display the tree" << endl;
        // cout << "5. Exit" << endl;
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1:
                cout << "Enter key to insert: ";
                cin >> key;
                t.insertion(key);
                cout << "Key inserted successfully!" << endl;
                break;
            case 2:
                cout << "Enter key to search: ";
                cin >> key;
                if (t.search(key)) {
                    cout << "Key " << key << " found in the tree." << endl;
                } else {
                    cout << "Key " << key << " not found in the tree." << endl;
                }
                break;
            case 3:
                cout << "Enter key to delete: ";
                cin >> key;
                t.deletion(key);
                cout << "Key deleted successfully!" << endl;
                break;
            case 4:
                cout << "Traversal of the tree: ";
                t.traversal();
                cout << endl;
                break;
            case 5:
                cout << "Exiting program. Goodbye!" << endl;
                return 0;
            default:
                cout << "Invalid choice! Please try again." << endl;
        }
    }

  return 0;
}


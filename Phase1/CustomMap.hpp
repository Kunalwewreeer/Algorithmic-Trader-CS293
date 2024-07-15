
#ifndef CUSTOM_MAP_HPP
#define CUSTOM_MAP_HPP

#include <iostream>

template <typename Key, typename Value>
struct Node
{
    Key key;
    Value value;
    Node *parent, *left, *right;
    int color; // 0 for black, 1 for red

    Node() : parent(nullptr), left(nullptr), right(nullptr), color(0) {}
    Node(Key k, Value v) : key(k), value(v), parent(nullptr), left(nullptr), right(nullptr), color(1) {}
};

template <typename Key, typename Value>
using NodePtr = Node<Key, Value> *;

template <typename Key, typename Value>
class CustomMap
{
private:
    NodePtr<Key, Value> root;
    NodePtr<Key, Value> TNULL;

    // Helper functions declaration
    NodePtr<Key, Value> findNode(NodePtr<Key, Value> node, const Key &key) const;
    void deleteNodeHelper(NodePtr<Key, Value> node, const Key &key);
    void insertFix(NodePtr<Key, Value> k);
    void rbTransplant(NodePtr<Key, Value> u, NodePtr<Key, Value> v);
    void deleteFix(NodePtr<Key, Value> x);
    void leftRotate(NodePtr<Key, Value> x);
    void rightRotate(NodePtr<Key, Value> x);
    NodePtr<Key, Value> minimum(NodePtr<Key, Value> node) const
    {
        while (node->left != TNULL)
        {
            node = node->left;
        }
        return node;
    }

public:
    CustomMap()
    {
        TNULL = new Node<Key, Value>();
        TNULL->color = 0;
        TNULL->left = nullptr;
        TNULL->right = nullptr;
        root = TNULL;
    }

    void insert(Key key, Value value);
    void deleteNode(const Key &key);
    Value &operator[](const Key &key);
    NodePtr<Key, Value> find(const Key &key);
    void erase(const Key &key)
    {
        deleteNode(key);
    }
};

// Implementation of private helper functions
template <typename Key, typename Value>
NodePtr<Key, Value> CustomMap<Key, Value>::findNode(NodePtr<Key, Value> node, const Key &key) const
{
    if (node == TNULL || key == node->key)
    {
        return node;
    }

    if (key < node->key)
    {
        return findNode(node->left, key);
    }
    else
    {
        return findNode(node->right, key);
    }
    return nullptr;
}

template <typename Key, typename Value>
void CustomMap<Key, Value>::deleteNodeHelper(NodePtr<Key, Value> node, const Key &key)
{
    NodePtr<Key, Value> z = TNULL;
    NodePtr<Key, Value> x, y;

    while (node != TNULL)
    {
        if (node->key == key)
        {
            z = node;
        }

        if (node->key <= key)
        {
            node = node->right;
        }
        else
        {
            node = node->left;
        }
    }

    if (z == TNULL)
    {
        return;
    }

    y = z;
    int y_original_color = y->color;
    if (z->left == TNULL)
    {
        x = z->right;
        rbTransplant(z, z->right);
    }
    else if (z->right == TNULL)
    {
        x = z->left;
        rbTransplant(z, z->left);
    }
    else
    {
        y = minimum(z->right);
        y_original_color = y->color;
        x = y->right;
        if (y->parent == z)
        {
            x->parent = y;
        }
        else
        {
            rbTransplant(y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }

        rbTransplant(z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }
    delete z;
    if (y_original_color == 0)
    {
        deleteFix(x);
    }
}

// ... implementation of other helper functions ...

// Implementation of public member functions
template <typename Key, typename Value>
void CustomMap<Key, Value>::insert(Key key, Value value)
{
    Node<Key, Value> *node = new Node<Key, Value>(key, value);
    node->parent = nullptr;
    node->left = TNULL;
    node->right = TNULL;
    node->color = 1; // New node must be red

    Node<Key, Value> *y = nullptr;
    Node<Key, Value> *x = this->root;

    while (x != TNULL)
    {
        y = x;
        if (node->key < x->key)
        {
            x = x->left;
        }
        else
        {
            x = x->right;
        }
    }

    // y is parent of x
    node->parent = y;
    if (y == nullptr)
    {
        root = node;
    }
    else if (node->key < y->key)
    {
        y->left = node;
    }
    else
    {
        y->right = node;
    }

    // If new node is a root node, simply return
    if (node->parent == nullptr)
    {
        node->color = 0;
        return;
    }

    // If the grandparent is null, simply return
    if (node->parent->parent == nullptr)
    {
        return;
    }

    // Fix the tree
    insertFix(node);
}

template <typename Key, typename Value>
void CustomMap<Key, Value>::deleteNode(const Key &key)
{
    deleteNodeHelper(this->root, key);
}

template <typename Key, typename Value>
Value &CustomMap<Key, Value>::operator[](const Key &key)
{
    NodePtr<Key, Value> node = findNode(this->root, key);

    // If the key is found, return a reference to its value
    if (node != TNULL)
    {
        return node->value;
    }

    // If not found, insert a new node with the key and a default-constructed value
    insert(key, Value());
    return findNode(this->root, key)->value;
}

template <typename Key, typename Value>
NodePtr<Key, Value> CustomMap<Key, Value>::find(const Key &key)
{
    return findNode(this->root, key);
}
template <typename Key, typename Value>
void CustomMap<Key, Value>::insertFix(NodePtr<Key, Value> k)
{
    NodePtr<Key, Value> u;
    while (k->parent->color == 1)
    {
        if (k->parent == k->parent->parent->right)
        {
            u = k->parent->parent->left; // uncle
            if (u->color == 1)
            {
                u->color = 0;
                k->parent->color = 0;
                k->parent->parent->color = 1;
                k = k->parent->parent;
            }
            else
            {
                if (k == k->parent->left)
                {
                    k = k->parent;
                    rightRotate(k);
                }
                k->parent->color = 0;
                k->parent->parent->color = 1;
                leftRotate(k->parent->parent);
            }
        }
        else
        {
            u = k->parent->parent->right; // uncle
            if (u->color == 1)
            {
                u->color = 0;
                k->parent->color = 0;
                k->parent->parent->color = 1;
                k = k->parent->parent;
            }
            else
            {
                if (k == k->parent->right)
                {
                    k = k->parent;
                    leftRotate(k);
                }
                k->parent->color = 0;
                k->parent->parent->color = 1;
                rightRotate(k->parent->parent);
            }
        }
        if (k == root)
        {
            break;
        }
    }
    root->color = 0;
}

template <typename Key, typename Value>
void CustomMap<Key, Value>::rbTransplant(NodePtr<Key, Value> u, NodePtr<Key, Value> v)
{
    if (u->parent == nullptr)
    {
        root = v;
    }
    else if (u == u->parent->left)
    {
        u->parent->left = v;
    }
    else
    {
        u->parent->right = v;
    }
    v->parent = u->parent;
}

template <typename Key, typename Value>
void CustomMap<Key, Value>::deleteFix(NodePtr<Key, Value> x)
{
    NodePtr<Key, Value> s;
    while (x != root && x->color == 0)
    {
        if (x == x->parent->left)
        {
            s = x->parent->right;
            if (s->color == 1)
            {
                s->color = 0;
                x->parent->color = 1;
                leftRotate(x->parent);
                s = x->parent->right;
            }

            if (s->left->color == 0 && s->right->color == 0)
            {
                s->color = 1;
                x = x->parent;
            }
            else
            {
                if (s->right->color == 0)
                {
                    s->left->color = 0;
                    s->color = 1;
                    rightRotate(s);
                    s = x->parent->right;
                }

                s->color = x->parent->color;
                x->parent->color = 0;
                s->right->color = 0;
                leftRotate(x->parent);
                x = root;
            }
        }
        else
        {
            s = x->parent->left;
            if (s->color == 1)
            {
                s->color = 0;
                x->parent->color = 1;
                rightRotate(x->parent);
                s = x->parent->left;
            }

            if (s->right->color == 0 && s->left->color == 0)
            {
                s->color = 1;
                x = x->parent;
            }
            else
            {
                if (s->left->color == 0)
                {
                    s->right->color = 0;
                    s->color = 1;
                    leftRotate(s);
                    s = x->parent->left;
                }

                s->color = x->parent->color;
                x->parent->color = 0;
                s->left->color = 0;
                rightRotate(x->parent);
                x = root;
            }
        }
    }
    x->color = 0;
}
template <typename Key, typename Value>
void CustomMap<Key, Value>::leftRotate(NodePtr<Key, Value> x)
{
    NodePtr<Key, Value> y = x->right; // Set y
    x->right = y->left;               // Turn y's left subtree into x's right subtree

    if (y->left != TNULL)
    {
        y->left->parent = x;
    }

    y->parent = x->parent; // Link x's parent to y

    if (x->parent == nullptr)
    {
        this->root = y;
    }
    else if (x == x->parent->left)
    {
        x->parent->left = y;
    }
    else
    {
        x->parent->right = y;
    }

    y->left = x; // Put x on y's left
    x->parent = y;
}

template <typename Key, typename Value>
void CustomMap<Key, Value>::rightRotate(NodePtr<Key, Value> x)
{
    NodePtr<Key, Value> y = x->left; // Set y
    x->left = y->right;              // Turn y's right subtree into x's left subtree

    if (y->right != TNULL)
    {
        y->right->parent = x;
    }

    y->parent = x->parent; // Link x's parent to y

    if (x->parent == nullptr)
    {
        this->root = y;
    }
    else if (x == x->parent->right)
    {
        x->parent->right = y;
    }
    else
    {
        x->parent->left = y;
    }

    y->right = x; // Put x on y's right
    x->parent = y;
}

#endif // CUSTOM_MAP_HPP
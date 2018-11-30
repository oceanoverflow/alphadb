#include "b_plus_tree.h"

b_plus_tree::b_plus_tree(const index_handle& ifh) : root_{nullptr}, index_handle_{ifh}, fanout_{0xFFFF}
{
    index_header hdr = index_handle_.index_header_;
    if (hdr.height > 0) {
        if (hdr.height == 1) 
            root_ = new leaf_node(nullptr, rid{hdr.root_page, -1}, this);
        else 
            root_ = new internal_node(nullptr, rid{hdr.root_page, -1}, this);
    }
}

b_plus_tree::~b_plus_tree() {
    delete root_;
    // dispose pages???
}

bool b_plus_tree::is_empty() const
{
    return root_ == nullptr;
}

rid b_plus_tree::find(key_t k) const
{
    leaf_node* node = find_leaf_node(k);
    int i = node->index_format_.find_in_leaf(k);
    
    if (i == -1) // If no search-key with value V is found in the leaf node
        return rid{-1, -1}; // return null to indicate failure
    else // TODO maybe more than one matching record, the remaining records also need to be fetched.
        return node->index_format_.rids[i];
}

std::vector<rid> b_plus_tree::find_all(key_t k) const
{
    // TODO consider supporting iterator interface, turn to index_scanner?
}

// procedure insert(value K, pointer P)
void b_plus_tree::insert(key_t k, const rid& p)
{
    // if tree is empty create an empty leaf node L, which is also the root
    if (is_empty()) {
        index_header hdr = index_handle_.index_header_;
        int page = index_handle_.allocate_page();
        hdr.root_page = page;
        hdr.height = 1;
        root_ = new leaf_node(nullptr, rid{hdr.root_page, -1}, this);
        static_cast<leaf_node *>(root_)->insert(k, p);    
    }
    else {
        // find the leaf node L that should contain key value K
        leaf_node* l = find_leaf_node(k);
        if (l == nullptr) throw std::exception{};
        
        // if L has less than n-1 key values
        if (l->size < l->max_size())
            insert_in_leaf(l, k, p);
        else { // L has n-1 key values already, split it
            // TODO do that in the page handle
            leaf_node* newleaf = split(l);
            // Set L'.Pn = L.Pn
            newleaf->set_next(l->next());
            // Set L.Pn = L'
            l->set_next(newleaf);
            // Copy T.P1 through T.K[n/2] from T into L starting at L.P1
            // Copy T.P[n/2+1] through T.Kn from T into L' starting at L'.P1
            // Let K' be the smallest key-value in L'
            key_t newkey = newleaf->first_key();
            // insert_in_parent(L, K', L')
            insert_in_parent(l, newkey, newleaf);
        }    
    }
}

// procedure insert_in_leaf(node L, value K, pointer P)
void b_plus_tree::insert_in_leaf(leaf_node* l, key_t k, rid p)
{
    l->insert(k, p);
}

// TODO what about the page handle of the changed root node
void b_plus_tree::adjust_root()
{
    // if (N is the root and N has only one remaining child)
    if (!root_->is_leaf() && root_->size() == 1) {
        // make the child of N the new root of the tree and delete N
        auto discarded_node = static_cast<internal_node *>(root_);
        root_ = static_cast<internal_node *>(root_)->remove_and_return_only_child();
        root_->set_parent(nullptr);
        delete discarded_node;
    }
    else if (root_->size() == 0) {
       delete root_;
       root_ = nullptr; 
    }
}

// procedure insert_in_parent(node N, value K', node N')
void b_plus_tree::insert_in_parent(node* n, key_t k, node* n1)
{
    // if (N is the root of the tree)
    if (n == root_) {
        int page = index_handle_.allocate_page();
        // Make R the root of the tree
        root_ = new internal_node(rid{page, -1}, this);
        n->set_parent(root_);
        n1->set_parent(root_);
        // Create a new node R containing N, K', N /* N and N' are pointers */
        static_cast<internal_node *>(root_)->populate_new_root(n, k, n1);
    }
    
    // Let P = parent(N)
    internal_node* parent = static_cast<internal_node *>(n->parent());

    // if (P has less than n pointers)
    if (parent->size() < parent->max_size())
        // insert (K', N') in P' just after N
        parent->insert_node_after(n, k, n1);
    else { /* Split P */
        internal_node* newnode = split(parent);
        // TODO!!! insert value
        // Copy T.P1 . . . T.P[n/2]into P 
        // Let K'' = T.K[n/2] 
        // Copy T.P[n/2]+1...T.Pn+1 into P' 
        key_t newkey = newnode->replace_and_return_first_key();
        // insert_in_parent(P, K'', P')
        insert_in_parent(parent, newkey, newnode);
    }
}

// procedure delete(value K, pointer P)
bool b_plus_tree::remove(key_t k, const rid& p)
{
    if (is_empty()) return false;
    // find the leaf node L that contains (K, P)
    leaf_node* l = find_leaf_node(k);
    // delete_entry(L, K, P)
    return remove_entry(l, k, p);    
}

// procedure delete_entry(node N, value K, pointer P)
bool b_plus_tree::remove_entry(leaf_node* l, key_t k, rid p)
{
    // TODO rid should have a valid null value
    if (l->lookup(k) != p) return false;
    // delete (K, P) from N
    int newsize = l->remove(k, p);   
    // if (N has too few value/pointers)     
    if (newsize < l->min_size())
        coalesce_or_redistribute(l); // ensure that each node is at least half-full
    return true;
}

template<typename N>
N* b_plus_tree::split(N* n)
{
    // Create node L'
    int page = index_handle_.allocate_page();
    page_handle ph = index_handle_.get_this_page(page);
    N* newnode = new N(n->parent(), rid{page, -1}, this);
    // leaf node split: put the first ⌈n/2⌉ in the existing node and the remaining values in a newly created node
    n->move_half_to(newnode);
    return newnode;
}

template<typename N>
void b_plus_tree::coalesce_or_redistribute(N* n)
{ 
    // if (N is the root and N has only one remaining child)
    if (n->is_root()) { // This deletion can results in recursive application of the deletion algorithm until the root is reached
        adjust_root();
        return;
    }

    auto parent = static_cast<internal_node *>(n->parent());
    // TODO find the index of node in the parent;
    int index_of_node_in_parent = 0;
    int neighbor_index = index_of_node_in_parent == 0 ? 1 : index_of_node_in_parent - 1;
    // Let N' be the previous or next child of parent(N)
    // Let K' be the value between pointers N and N's in parent(N)
    N* neighbor_node = static_cast<N*>(parent->neighbor(neighbor_index));
    // if (entries in N and N's can fit in a single node)
    if (n->size() + neighbor_node->size() <= neighbor_node->max_size()) /* Coalesce nodes */ 
        coalesce(neighbor_node, n, static_cast<internal_node *>(n->parent()), index_of_node_in_parent);
    else /* Redistribution: borrow an entry from N' */
        redistribute(neighbor_node, n, static_cast<internal_node *>(n->parent()), index_of_node_in_parent);
}

template<typename N>
void b_plus_tree::coalesce(N* neighbor, N* n, internal_node* parent, int index)
{
    // if (N is a predecessor of N') then swap_variables(N, N')
    if (index == 0) {
        std::swap(neighbor, n);
        index = 1;
    }
    /*
    if (N is not a leaf)
        then append K' and all pointers and values in N to N'
    else
        append all (Ki, Pi) pairs in N to N'; set N'.Pn = N.Pn
    end
    */
    n->move_all_to(neighbor, index);
    // delete_entry(parent(N), K', N)
    parent->remove(index);
    
    if (parent->size < parent->min_size())
        coalesce_or_redistribute(parent);
    
    // delete node N
    index_handle_.dispose_page(n.page_id_.page());
    delete n;
}

template<typename N>
void b_plus_tree::redistribute(N* neighbor, N* n, internal_node* parent, int index)
{
    if (index != 0) // if (N' is a predecessor of N)
        neighbor->move_last_to_front_of(n, index);
    else
        neighbor->move_first_to_end_of(n);
}

void b_plus_tree::destroy()
{
    if (root_->is_leaf())
        delete static_cast<leaf_node *>(root_);
    else
        delete static_cast<internal_node *>(root_);

    root_ = nullptr;
    // index_handle_.dispose_page();
}

// node* fetch_node(int page);
node* b_plus_tree::fetch_node(int page)
{
    index_handle_.get_this_page(page);
    return new node(rid{page, -1}, this);
}

leaf_node* b_plus_tree::find_leaf_node(key_t k) const
{
    if (is_empty()) return nullptr;

    // set C = root node, C stands for current node
    auto n = root_;  
    if (height_ == 1) 
        return static_cast<leaf_node *>(root_);

    // A B+-tree index takes the form of a balanced tree
    // in which every path from the root of the tree to
    // a leaf of the tree is of the same length
    for(size_t i = 0; i < height_; i++)
    {
        auto internalnode = static_cast<internal_node *>(n);
        index_handle_.get_this_page(internalnode->page_id_.page());
        // Let Ki = smallest search-key value, if any, greater than or equal to V
        n = internalnode->lookup(k);

        // traversing down the tree until a leaf node is reached
        if (i == height_ -1)
            return static_cast<leaf_node *>(n);   

        // while C is not root
        if (i != 0)
            index_handle_.unpin_page(internalnode->page_id_.page());     
    }
}

// TODO: be careful of memory leak.
leaf_node* b_plus_tree::find_smallest_leaf_node() const
{
    if (is_empty()) return nullptr;

    auto n = root_;
    if (height_ == 1)
        return static_cast<leaf_node *>(root_);
    
    for(size_t i = 0; i < height_; i++)
    {
        auto internalnode = static_cast<internal_node *>(n);
        index_handle_.get_this_page(internalnode->page_id_.page());

        rid id = internalnode->index_format_.rids[0];
        n = new node(id, this);

        if (i == height_ - 1)
            return static_cast<leaf_node *>(n);

        if (i != 0)
            index_handle_.unpin_page(internalnode->page_id_.page()); 
    }
}

// TODO: be careful of memory leak.
leaf_node* b_plus_tree::find_largest_leaf_node() const
{
    if (is_empty()) return nullptr;

    auto n = root_;
    if (height_ == 1)
        return static_cast<leaf_node *>(root_);

    for(size_t i = 0; i < height_; i++)
    {
        auto internalnode = static_cast<internal_node *>(n);
        index_handle_.get_this_page(internalnode->page_id_.page());

        rid id = internalnode->index_format_.rids[internalnode->size()-1];
        n = new node(id, this);

        if (i == height_ - 1)
            return static_cast<leaf_node *>(n);

        if (i != 0)
            index_handle_.unpin_page(internalnode->page_id_.page());
    }
}

bool b_plus_tree::update(key_t k, const rid& oldp, const rid& newp)
{   
    if (find(k) == rid{-1, -1}) return false;
    if (remove(k, oldp) == false) return false;
    insert(k, newp); 
}
#include "index_handle.h"

index_handle::index_handle(file_handle& fh, index_header header)
: file_handle_{fh}, index_header_{header}, file_is_open_{true}, header_changed_{false}
{
    tree_ = new b_plus_tree(*this);
}

index_handle::~index_handle() {}

rid index_handle::search(void* data)
{
    return this->tree_->find(key_t(data));
}

void index_handle::insert_entry(void* data, const rid& id)
{
    this->tree_->insert(key_t(data), id);
}

bool index_handle::delete_entry(void* data, const rid& id)
{
    // this->tree_->remove(key_t(data), rid);
    return this->tree_->remove(key_t(data), id);
}

int index_handle::allocate_page()
{
    page_handle ph = file_handle_.allocate_page();
    return ph.page();
}

void index_handle::dispose_page(int page)
{
    file_handle_.dispose_page(page);
    index_header_.num_pages--;
    assert(index_header_.num_pages > 0);
    header_changed_ = true;
}

void index_handle::pin_page(int page)
{
    file_handle_.get_this_page(page);
}

void index_handle::unpin_page(int page)
{
    file_handle_.unpin_page(0);
}

void index_handle::force_pages()
{
    file_handle_.force_page(ALL_PAGES);
}

void index_handle::mark_dirty(int page)
{
    file_handle_.mark_dirty(page);
}
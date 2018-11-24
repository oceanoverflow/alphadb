#include "rid.h"

rid::rid(): page_{NULL_PAGE}, slot_{NULL_SLOT} {}

rid::rid(int page, int slot): page_{page}, slot_{slot} {}

rid::~rid() {}

rid::rid(const rid& id)
{
    page_ = id.page_;
    slot_ = id.slot_;
}

rid& rid::operator=(const rid& id)
{ 
    if (this != &id) {
        page_ = id.page_;
        slot_ = id.slot_;
    }
    return *this;
}

int rid::page() const
{
    return page_;
}

int rid::slot() const
{
    return slot_;
}

bool rid::operator==(const rid& id)
{
    return page_ == id.page_ && slot_ == id.slot_;
}

bool rid::operator!=(const rid& id)
{
    return !(*this == id);
}

std::ostream& operator<<(std::ostream& os, const rid& id)
{
    os << "rid<" << id.page_ << "," << id.slot_ << ">" << std::endl;
    return os;
}

void rid::set(int page, int slot)
{
    page_ = page;
    slot_ = slot;
}

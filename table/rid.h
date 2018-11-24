#pragma once

#include <iostream>

const int NULL_PAGE{-1};
const int NULL_SLOT{-1};

class rid
{
    friend std::ostream& operator<<(std::ostream& os, const rid& id);
private:
    int page_;
    int slot_;
public:
    rid();
    rid(int page, int slot);
    rid(const rid& id);
    rid& operator=(const rid& id);
    ~rid();

    int page() const;
    int slot() const;

    void set(int page, int slot);

    bool operator==(const rid& id);
    bool operator!=(const rid& id);
};



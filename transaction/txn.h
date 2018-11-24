#pragma once

typedef uint64_t txn_id_t;

class txn
{
private:
    
public:
    txn();
    ~txn();

    virtual int rtti() const = 0;

    virtual void abort() = 0;
    virtual bool commit() = 0;

};


class unsafe_txn: public txn
{
private:
    
public:
    unsafe_txn();
    ~unsafe_txn();

    virtual int rtti() const { return 1; }

    void abort() override {}
    bool commit() override { return true; }
    
};


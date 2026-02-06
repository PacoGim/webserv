#include "SharedData.hpp"

SharedData::SharedData(AData *p):
    ptr(p),
    count(p ? new size_t(1) : NULL)
{}

SharedData::SharedData(const SharedData& other):
ptr(other.ptr),
count(other.count)
{ if (count) ++(*count); }

SharedData& SharedData::operator=(const SharedData& other)
{
    if (this != &other)
    {
        release();
        ptr = other.ptr;
        count = other.count;
        if (count) ++(*count);
    }
    return *this;
}

SharedData::~SharedData() { release(); }

AData* SharedData::operator->() { return ptr; }
const AData* SharedData::operator->() const { return ptr; }

AData& SharedData::operator*() { return *ptr; }
const AData& SharedData::operator*() const { return *ptr; }

AData* SharedData::get() const { return ptr; }

void SharedData::release()
{
    if (count)
    {
        if (--(*count) == 0)
        {
            ptr->clean_memory();
            delete ptr;
            delete count;
        }
    }
    ptr = NULL;
    count = NULL;
}
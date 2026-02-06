#ifndef VOGO_WEBSERV_SHARED_DATA_HPP
#define VOGO_WEBSERV_SHARED_DATA_HPP

#include "DataCached.hpp"

#include <cstddef>

class SharedData
{

public:
    explicit SharedData(AData *p = NULL);

    SharedData(const SharedData& other);

    SharedData& operator=(const SharedData& other);

    ~SharedData();

    AData* operator->();
    const AData* operator->() const;

    AData& operator*();
    const AData& operator*() const;

    AData* get() const;

private:
    AData *ptr;
    size_t *count;

    void release();
};

#endif //VOGO_WEBSERV_SHARED_DATA_HPP
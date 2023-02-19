#pragma once

//! connection handlers used with server_t<> must implement this interface
class abstract_connection_t {
    public:
    virtual void start() = 0;
};
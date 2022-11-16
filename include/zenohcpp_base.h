#pragma once

#include <utility>

namespace zenoh {

template<typename ZC_OWNED_TYPE> class Owned {
public:
    Owned& operator=(const Owned& v) = delete;
    Owned(const Owned& v) = delete;
    Owned(ZC_OWNED_TYPE&& v) : _0(v) { v = {}; }
    Owned(Owned&& v) : Owned(std::move(v._0)) {}
    virtual ~Owned() { z_drop(&_0); }
    ZC_OWNED_TYPE take() { auto r = _0; _0 = {}; return r; }
    bool check() { return z_check(_0); }
protected:
    Owned() : _0({}) {};
    ZC_OWNED_TYPE _0;
};

template<typename ZC_CLOSURE_TYPE, typename ZC_CLOSURE_CALL_PARAM, typename ZCPP_CLOSURE_CALL_PARAM> class Closure : public Owned<ZC_CLOSURE_TYPE> {
public:
    using Owned<ZC_CLOSURE_TYPE>::Owned;
    template<typename LAMBDA> Closure(LAMBDA&& lambda) {
        this->_0.context = new LAMBDA(std::move(lambda));
        this->_0.drop = [](void* ctx){delete static_cast<LAMBDA*>(ctx);};
        this->_0.call = [](ZC_CLOSURE_CALL_PARAM* pvalue, void* ctx){ static_cast<LAMBDA*>(ctx)->operator()(ZCPP_CLOSURE_CALL_PARAM(std::move(*pvalue))); };
    }
};

}
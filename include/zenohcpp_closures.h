#pragma once

#include "zenoh.h"

#include <utility>

namespace zenoh {

template<typename ZC_CLOSURE_TYPE, typename ZC_CLOSURE_CALL_PARAM, typename ZCPP_CLOSURE_CALL_PARAM> class Closure {
public:
    template<typename LAMBDA> Closure(LAMBDA&& lambda) {
        zclosure.context = new LAMBDA(std::move(lambda));
        zclosure.drop = [](void* ctx){delete static_cast<LAMBDA*>(ctx);};
        zclosure.call = [](ZC_CLOSURE_CALL_PARAM* pvalue, void* ctx){ static_cast<LAMBDA*>(ctx)->operator()(ZCPP_CLOSURE_CALL_PARAM(std::move(*pvalue))); };
    }
    Closure(const Closure&) = delete;
    Closure(Closure&& v) {
        this->zclosure = v.zclosure;
        v.zclosure = {};
    }
    ~Closure() {
        if (zclosure.drop) {
            zclosure.drop(zclosure.context);
        }
    }
    ZC_CLOSURE_TYPE take() { auto v = zclosure; zclosure = {}; return v; }
private:
    ZC_CLOSURE_TYPE zclosure; 
};

typedef Closure<::z_owned_closure_reply_t, ::z_owned_reply_t, Reply> ClosureReply;

}
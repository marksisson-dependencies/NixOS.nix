#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "libexprtests.hh"

namespace nix {

    using namespace testing;

    // Testing eval of PrimOp's
    class ErrorTraceTest : public LibExprTest { };

#define ASSERT_TRACE1(args, type, message)                                  \
        ASSERT_THROW(                                                       \
            try {                                                           \
                eval("builtins." args);                                     \
            } catch (BaseError & e) {                                       \
                ASSERT_EQ(PrintToString(e.info().msg),                      \
                          PrintToString(message));                          \
                auto trace = e.info().traces.rbegin();                      \
                ASSERT_EQ(PrintToString(trace->hint),                       \
                          PrintToString(hintfmt("while calling the '%s' builtin", "genericClosure"))); \
                throw;                                                      \
            }                                                               \
            , type                                                          \
        )

#define ASSERT_TRACE2(args, type, message, context)                         \
        ASSERT_THROW(                                                       \
            try {                                                           \
                eval("builtins." args);                                     \
            } catch (BaseError & e) {                                       \
                ASSERT_EQ(PrintToString(e.info().msg),                      \
                          PrintToString(message));                          \
                auto trace = e.info().traces.rbegin();                      \
                ASSERT_EQ(PrintToString(trace->hint),                       \
                          PrintToString(context));                          \
                ++trace;                                                    \
                ASSERT_EQ(PrintToString(trace->hint),                       \
                          PrintToString(hintfmt("while calling the '%s' builtin", "genericClosure"))); \
                throw;                                                      \
            }                                                               \
            , type                                                          \
        )

    TEST_F(ErrorTraceTest, genericClosure) { \
        ASSERT_TRACE2("genericClosure 1",
                      TypeError,
                      hintfmt("value is %s while a set was expected", "an integer"),
                      hintfmt("while evaluating the first argument passed to builtins.genericClosure"));

        ASSERT_TRACE1("genericClosure {}",
                      TypeError,
                      hintfmt("attribute '%s' missing %s", "startSet", normaltxt("in the attrset passed as argument to builtins.genericClosure")));

        ASSERT_TRACE2("genericClosure { startSet = 1; }",
                      TypeError,
                      hintfmt("value is %s while a list was expected", "an integer"),
                      hintfmt("while evaluating the 'startSet' attribute passed as argument to builtins.genericClosure"));

        // Okay:      "genericClosure { startSet = []; }"

        ASSERT_TRACE2("genericClosure { startSet = [{ key = 1;}]; operator = true; }",
                      TypeError,
                      hintfmt("value is %s while a function was expected", "a Boolean"),
                      hintfmt("while evaluating the 'operator' attribute passed as argument to builtins.genericClosure"));

        ASSERT_TRACE2("genericClosure { startSet = [{ key = 1;}]; operator = item: true; }",
                      TypeError,
                      hintfmt("value is %s while a list was expected", "a Boolean"),
                      hintfmt("while evaluating the return value of the `operator` passed to builtins.genericClosure")); // TODO: inconsistent naming

        ASSERT_TRACE2("genericClosure { startSet = [{ key = 1;}]; operator = item: [ true ]; }",
                      TypeError,
                      hintfmt("value is %s while a set was expected", "a Boolean"),
                      hintfmt("while evaluating one of the elements generated by (or initially passed to) builtins.genericClosure"));

        ASSERT_TRACE1("genericClosure { startSet = [{ key = 1;}]; operator = item: [ {} ]; }",
                      TypeError,
                      hintfmt("attribute '%s' missing %s", "key", normaltxt("in one of the attrsets generated by (or initially passed to) builtins.genericClosure")));

        ASSERT_TRACE2("genericClosure { startSet = [{ key = 1;}]; operator = item: [{ key = ''a''; }]; }",
                      EvalError,
                      hintfmt("cannot compare %s with %s", "a string", "an integer"),
                      hintfmt("while comparing the `key` attributes of two genericClosure elements"));

        ASSERT_TRACE2("genericClosure { startSet = [ true ]; operator = item: [{ key = ''a''; }]; }",
                      TypeError,
                      hintfmt("value is %s while a set was expected", "a Boolean"),
                      hintfmt("while evaluating one of the elements generated by (or initially passed to) builtins.genericClosure"));

    }

} /* namespace nix */
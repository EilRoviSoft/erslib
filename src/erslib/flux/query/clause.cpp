#include "erslib/flux/query/clause.hpp"

flux::impl::IClause::IClause(SlotRef slot) :
    _slot(slot) {
}

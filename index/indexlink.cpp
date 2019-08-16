#include "indexlink.h"
#include <opencog/atoms/atom_types/NameServer.h>
#include <opencog/atoms/core/UnorderedLink.h>
#include <opencog/atomspace/AtomSpace.h>
#include <opencog/query/DefaultImplicator.h>

IndexLink::IndexLink(const Handle& vardecl,
                   const Handle& body,
                   const Handle& rewrite, std::shared_ptr<Index> idx)
    : BindLink(HandleSeq{vardecl, body, rewrite}),
      index(idx)
{}

IndexLink::IndexLink(const Handle& body, const Handle& rewrite, std::shared_ptr<Index> idx)
    : BindLink(HandleSeq{body, rewrite}),
      index(idx)
{}

IndexLink::IndexLink(const HandleSeq& hseq,  Type t, std::shared_ptr<Index> idx)
    : BindLink(hseq, t),
      index(idx)
{}

ValueSet QueryLink::do_execute(AtomSpace* as, bool silent)
{
    if (nullptr == as) as = _atom_space;

    DefaultImplicator impl(as);
    impl.implicand = this->get_implicand();

    /*
     * The `do_conn_check` flag stands for "do connectivity check"; if the
     * flag is set, and the pattern is disconnected, then an error will be
     * thrown. The URE explicitly allows disconnected graphs.
     *
     * Set the default to always allow disconnected graphs. This will
     * get naive users into trouble, but there are legit uses, not just
     * in the URE, for doing disconnected searches.
     */
    bool do_conn_check=false;
    if (do_conn_check and 0 == _virtual.size() and 1 < _components.size())
        throw InvalidParamException(TRACE_INFO,
                                    "QueryLink consists of multiple "
                                    "disconnected components!");

    this->PatternLink::satisfy(impl);

    // If we got a non-empty answer, just return it.
    if (0 < impl.get_result_set().size())
    {
        // The result_set contains a list of the grounded expressions.
        // (The order of the list has no significance, so it's really a set.)
        return impl.get_result_set();
    }

    // If we are here, then there were zero matches.
    //
    // There are certain useful queries, where the goal of the query
    // is to determine that some clause or set of clauses are absent
    // from the AtomSpace. If the clauses are jointly not found, after
    // a full and exhaustive search, then we want to run the implicator,
    // and perform some action. Easier said than done, this code is
    // currently a bit of a hack. It seems to work, per the AbsentUTest
    // but is perhaps a bit fragile in its assumptions.
    //
    // Theoretical background: the atomspace can be thought of as a
    // Kripke frame: it holds everything we know "right now". The
    // AbsentLink is a check for what we don't know, right now.
    const Pattern& pat = this->get_pattern();
    DefaultPatternMatchCB* intu =
        dynamic_cast<DefaultPatternMatchCB*>(&impl);
    if (0 == pat.mandatory.size() and 0 < pat.optionals.size()
        and not intu->optionals_present())
    {
        ValueSet result;
        result.insert(impl.inst.execute(impl.implicand, true));
        return result;
    }

    return ValueSet();
}

/** Wrap query results in a SetLink, place them in the AtomSpace. */
ValuePtr BindLink::execute(AtomSpace* as, bool silent)
{
    // The result_set contains a list of the grounded expressions.
    // (The order of the list has no significance, so it's really a set.)
    // Put the set into a SetLink, cache it, and return that.
    ValueSet rslt(do_execute(as, silent));
    HandleSeq hlist;
    for (const ValuePtr& v: rslt) hlist.emplace_back(HandleCast(v));
    Handle rewr(createUnorderedLink(hlist, SET_LINK));
    return rewr;
}

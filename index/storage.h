#ifndef STORAGE_H
#define STORAGE_H
#include <cstddef>
#include <utility>
#include <experimental/optional>
#include <opencog/atoms/base/Handle.h>
#include <opencog/atomspace/AtomSpace.h>

typedef std::size_t atomid;
typedef std::optional<atomid> optionalid;


typedef std::iterator<std::input_iterator_tag, atomid> Atomiterator;

class Storage
{
public:
    virtual std::pair<atomid, bool> add_atom(opencog::Handle const & atom) = 0;
    virtual optionalid find_id_by_atom(opencog::Handle const & atom) = 0;
    virtual opencog::Handle get_atom(opencog::AtomSpace&, atomid) = 0;
    virtual std::vector<opencog::Handle> get_atoms(opencog::AtomSpace&, std::vector<atomid> const &)=0;
    virtual std::set<opencog::Handle> get_atoms(opencog::AtomSpace&, std::set<atomid> const &)=0;
};

#endif // STORAGE_H

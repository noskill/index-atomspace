#ifndef STORAGE_H
#define STORAGE_H
#include <cstddef>
#include <utility>
#include <experimental/optional>
#include <opencog/atoms/base/Handle.h>

typedef std::size_t atomid;
typedef std::optional<atomid> optionalid;


typedef std::iterator<std::input_iterator_tag, atomid> Atomiterator;

class Storage
{
public:
    virtual std::pair<atomid, bool> add_atom(opencog::Handle & atom) = 0;
    virtual optionalid find_id_by_atom(opencog::Handle & atom) = 0;
    virtual opencog::Handle get_atom(atomid) = 0;
};

#endif // STORAGE_H

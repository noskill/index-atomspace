#ifndef MYSTORAGE_H
#define MYSTORAGE_H

#include <map>
#include "storage.h"

class MyStorage : public Storage
{
private:
    std::vector<std::string> slow_storage;
    std::map<opencog::ContentHash, std::vector<atomid>> hash_to_idx;
public:
    MyStorage()=default;
    std::pair<atomid, bool> add_atom(opencog::Handle const & atom);
    optionalid find_id_by_atom(opencog::Handle const & atom);
    virtual opencog::Handle get_atom(opencog::AtomSpace&, atomid);
    virtual std::vector<opencog::Handle> get_atoms(opencog::AtomSpace&, std::vector<atomid> const &);
    virtual std::set<opencog::Handle> get_atoms(opencog::AtomSpace&, std::set<atomid> const&);
};

#endif // MYSTORAGE_H

#ifndef MYSTORAGE_H
#define MYSTORAGE_H

#include <map>
#include "storage.h"

class MyStorage : public Storage
{
private:
    std::vector<opencog::Handle> slow_storage;
    std::map<opencog::ContentHash, std::vector<atomid>> hash_to_idx;
public:
    MyStorage()=default;
    std::pair<atomid, bool> add_atom(opencog::Handle & atom);
    optionalid find_id_by_atom(opencog::Handle & atom);
    opencog::Handle get_atom(atomid);
};

#endif // MYSTORAGE_H

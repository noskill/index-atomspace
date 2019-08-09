#include "mystorage.h"
#include <chrono>
#include <thread>
#include <experimental/optional>
#include <opencog/atoms/base/Atom.h>

std::pair<atomid, bool> MyStorage::add_atom(opencog::Handle & atom){
    if (auto id = find_id_by_atom(atom)){
        std::make_pair(*id, false);
    }
    atomid id = slow_storage.size();
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(1s);
    slow_storage.push_back(atom);
    auto hash = atom.value();
    hash_to_idx[hash].push_back(id);
    return std::make_pair(id, true);
}

optionalid MyStorage::find_id_by_atom(opencog::Handle & atom){
    auto it = hash_to_idx.find(atom->get_hash());
    if (it == hash_to_idx.end()){
        return std::nullopt;
    }
    std::vector<atomid> & ids = it->second;
    if(ids.empty()){
        return std::nullopt;
    }
    for(atomid id: ids){
        if (atom == slow_storage[id]){
            return id;
        }
    }
    return std::nullopt;
}


opencog::Handle MyStorage::get_atom(atomid id){
   return slow_storage[id];
}

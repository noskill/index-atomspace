import uuid
from opencog.atomspace import AtomSpace, types, create_child_atomspace
from opencog.utilities import initialize_opencog, tmp_atomspace
from opencog.type_constructors import *
from opencog.scheme_wrapper import scheme_eval_h
from opencog.bindlink import execute_atom


def replace(atom, semantic_space, pattern_space, varmapping):
    if atom.is_link():
        args = [replace(x, semantic_space,
                        pattern_space, varmapping) for x in atom.out]
        return pattern_space.add_link(atom.type, args)
    assert atom.is_node()
    if atom in semantic_space:
        return pattern_space.add_atom(atom)
    if atom in varmapping:
        return varmapping[atom]
    new_varnode = pattern_space.add_node(types.VariableNode, '$X' + str(len(varmapping)))
    varmapping[atom] = new_varnode
    return new_varnode


def rename(tmp, atomspace, atom):
    if atom.is_link():
        args = [rename(tmp, atomspace, x) for x in atom.out]
        return tmp.add_link(atom.type, args)
    if atom.type == types.VariableNode:
        if atom in atomspace:
            # todo: check for random collision
            rnd_uuid = uuid.uuid1()
            name = atom.name + str(rnd_uuid)
            return tmp.add_node(atom.type, name)
    return atom



class Index:
    def __init__(self):
        self.pattern_space = AtomSpace()
        self.semantic_space = AtomSpace()
        self.data_space = AtomSpace()
        # index maps subgraph pattern to maching atoms
        self._index = dict()

    def add_toplevel_pattern(self, atom):
        args = [self.pattern_space.add_node(types.VariableNode, '$X' + str(i)) for i in
                                           range(atom.arity)]
        l = self.pattern_space.add_link(atom.type, args)
        bindlink = self.pattern_space.add_link(types.BindLink, [l, l])
        self.add_pattern(bindlink)

    def add_semantic_pattern(self, atom):
        # find all nodes, that inherit from some atom from semantic atomspace
        # replace such atoms by variable nodes

        result = replace(atom, self.semantic_space, self.pattern_space, dict())
        pattern = self.pattern_space.add_link(types.BindLink, [result, result])
        self.add_pattern(pattern)

    def add_data(self, data):
        data = self.data_space.add_atom(data)
        assert data.is_link()
        # self.add_toplevel_pattern(data)
        self.add_semantic_pattern(data)

        tmp = create_child_atomspace(self.pattern_space)
        content = tmp.add_atom(data)
        self.update_index(tmp, content, data)

    def update_index(self, atomspace, content, data):
        for pat in self._index.keys():
            match = execute_atom(atomspace, pat)
            # patterns can match patterns, so check:
            for m in match.out:
                # should work with atoms from different atomspaces after
                # merge, then it is possible to remove content
                if hash(m) == hash(content):
                    self._index[pat].add(data)

    def intersect(self, query):
        """
        Extracts relevant patterns to the query and intersects their indices
        returns set of atoms
        """
        # put query in tmp atomspace, check if there are relevant patterns
        tmp = create_child_atomspace(self.pattern_space)
        q = tmp.add_atom(rename(tmp, self.pattern_space, query))
        # check for exact match:
        exact_match = execute_atom(tmp, tmp.add_link(types.BindLink, [q, q]))
        for m in exact_match.out:
            return self._index[self.pattern_space.add_link(types.BindLink, [m, m])]
        # no exact match: search among all patterns
        # todo: search subgraphs
        res_set = None
        for pat, idx in self._index.items():
            match = execute_atom(tmp, pat)
            for m in match.out:
                if hash(m) == hash(q):
                    if res_set is None:
                        res_set = idx
                    else:
                        res_set = idx.intersection(idx)
        return res_set

    def query(self, query):
        res_set = self.intersect(query)
        # add all possible results to tmp atomspace and run query
        # what is called filter step in graph indexing literature
        tmp = AtomSpace()
        for item in res_set:
            tmp.add_atom(item)
        # pack query in BindLink
        q = tmp.add_link(types.BindLink, [query, query])
        return execute_atom(tmp, q)

    def update_index_dual_link(self, content):
        # doesn't work
        d = tmp.add_link(types.DualLink,
                [content])
        # result is a set of conditional parts from pattens(GetLinks)
        pattern_set = execute_atom(tmp, d)
        # todo: create a new pattern

        assert len(pattern_set.out)
        for pattern in pattern_set.out:
            get_link = self.pattern_space.add_link(types.GetLink, pattern)
            self._index[get_link].add(data)

    def add_pattern(self, pat):
        assert pat.type == types.BindLink
        print(hash(pat))
        atom = self.pattern_space.add_atom(pat)
        if atom not in self._index:
            self._index[atom] = set()


def main():
    path_query = 'query.scm'
    path_data = 'data.scm'
    atomspace = AtomSpace()
    initialize_opencog(atomspace)
    index = Index()
    with tmp_atomspace() as tmp:
        imp = ImplicationLink(
                VariableNode("C"),
                VariableNode("B"))
        # GetLink returns only bindings for variables
        pat = BindLink(imp, imp)
        index.add_pattern(pat)
    with tmp_atomspace() as tmp:
        base = open(path_data).read()
        data = scheme_eval_h(tmp, base)
        index.add_data(data)
        query = open(path_query).read()
        q = scheme_eval_h(tmp, query)
        result = index.query(q)
        print(result)

if __name__ == '__main__':
    main()


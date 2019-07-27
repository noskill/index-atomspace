from opencog.atomspace import AtomSpace, types, create_child_atomspace
from opencog.utilities import initialize_opencog, tmp_atomspace
from opencog.type_constructors import *
from opencog.scheme_wrapper import scheme_eval_h
from opencog.bindlink import execute_atom


def check_pattern(expression, pattern):
    # if patter matches query then it's relevant
    if sc is not None:
        match = execute_atom(tmp, pattern)
        if match.out:
            return True
    return False

class Index:
    def __init__(self):
        self.pattern_space = AtomSpace()
        self.data_space = AtomSpace()
        # index maps subgraph pattern to maching atoms
        self._index = dict()

    def add_data(self, data):
        tmp = create_child_atomspace(self.pattern_space)
        content = tmp.add_atom(data)
        self.update_index(tmp, content, data)

    def update_index(self, atomspace, content, data):
        for pat in self._index.keys():
            match = execute_atom(atomspace, pat)
            # patterns can match patterns, so check:
            for m in match.out:
                # todo: check if hashes from different atomspaces are same in c++
                # for python they are differen
                if hash(m) == hash(content):
                    self._index[pat].add(data)

    def intersect(self, query):
        """
        Extracts relevant patterns to the query and intersects their indices
        returns set of atoms
        """
        # put query in tmp atomspace, check if there are relevant patterns
        tmp = create_child_atomspace(self.pattern_space)
        q = tmp.add_atom(query)
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

        base = open(path_data).read()
        data = scheme_eval_h(tmp, base)
        index.add_data(data)
        query = open(path_query).read()
        q = scheme_eval_h(tmp, query)
        result = index.query(q)
        print(result)

if __name__ == '__main__':
    main()


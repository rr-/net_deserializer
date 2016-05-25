_GUARD = object()

class Node(dict):
    def __init__(self, name, children=_GUARD):
        super().__init__()
        self.name = name
        self.children = [] if children == _GUARD else children

    def add(self, node):
        self.children.append(node)

    def add_leaf(self, key, value=_GUARD):
        self.children.append(LeafNode(key, value))

    def get(self, key):
        filtered = [c for c in self.children if c.name == key]
        if filtered:
            return filtered[0]
        raise KeyError()

    def get_leaf(self, key, default_value=_GUARD):
        filtered = [c for c in self.children if c.name == key]
        if filtered:
            return filtered[0].value
        if default_value != _GUARD:
            return default_value
        raise KeyError()

class LeafNode(Node):
    def __init__(self, name, value=_GUARD):
        super().__init__(name)
        self.value = value

    def has_value(self):
        return self.value != _GUARD

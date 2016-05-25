#!/bin/python3
import argparse
import html
from net_deserializer.node import Node, LeafNode
from net_deserializer.records import deserialize

def dump(node, indent, prefix=''):
    if isinstance(node, LeafNode):
        if node.has_value():
            print(prefix + '<{name}>{value}</{name}>'.format(
                name=node.name, value=html.escape(str(node.value))))
        else:
            print(prefix + '<{name}/>'.format(name=node.name))
    elif isinstance(node, Node):
        print(prefix + '<{name}>'.format(name=node.name))
        for child in node.children:
            if child.name != 'MembersMeta':
                dump(child, indent, indent + prefix)
        print(prefix + '</{name}>'.format(name=node.name))
    else:
        print(node)
        assert False

def parse_args():
    parser = argparse.ArgumentParser(description='Deserialize .NET stream')
    parser.add_argument(
        metavar='PATH', dest='input_path',
        help='where to read the input data from')
    return parser.parse_args()

def main(args):
    with open(args.input_path, 'rb') as handle:
        root_node = deserialize(handle)
        dump(root_node, indent=' '*4)

if __name__ == '__main__':
    main(parse_args())

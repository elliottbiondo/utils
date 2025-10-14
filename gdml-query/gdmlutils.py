#!/usr/bin/env python
# Copyright Celeritas contributors: see top-level COPYRIGHT file for details
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
"""
GDML helper utilities.
"""

import re
from collections import defaultdict
import xml.etree.ElementTree as ET


class PointerReplacer:
    sub = re.compile(r"0x[0-9a-f]{4,}").sub

    def __init__(self):
        self.addrs = defaultdict(dict)

    def repl(self, match: re.Match):
        addr = match.group(0)

        # Get the prefix before the pointer in the original string
        prefix = match.string[: match.start()]
        prefix_addrs = self.addrs[prefix]

        # Get or create a new index
        idx = prefix_addrs.setdefault(addr, len(prefix_addrs))

        if idx == 0:
            return ""

        # Create the replacement string
        return f"@{idx:d}"

    def __call__(self, s):
        return self.sub(self.repl, s)


def multiunion_refs(node: ET.Element):
    for subel in node.iter("solid"):
        yield subel.attrib["ref"]


def boolean_refs(node: ET.Element):
    if node.tag in ("first", "second"):
        yield node.attrib["ref"]


def get_solid_refs(el: ET.Element):
    """Yield solid names referenced by a boolean/multiunion solid.
    """
    if el.tag == "multiUnion":
        get_node_refs = multiunion_refs
    elif el.tag in ("union", "subtraction", "intersection"):
        get_node_refs = boolean_refs
    else:
        return None
    
    for node in el:
        yield from get_node_refs(node)


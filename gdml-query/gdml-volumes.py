#!/usr/bin/env python
# Copyright Celeritas contributors: see top-level COPYRIGHT file for details
# SPDX-License-Identifier: (Apache-2.0 OR MIT)
"""
Count the number of logical and physical volumes in a GDML file.
"""

from itertools import count
from collections import namedtuple
import json
from pathlib import Path
import sys
import xml.etree.ElementTree as ET
from sys import stderr
from gdmlutils import get_solid_refs


def log(msg, *args, file=stderr, **kwargs):
    print(msg, *args, file=file, **kwargs)
    file.flush()


def get_material(volume_el):
    el = next(volume_el.iter("materialref"))
    return el.attrib["ref"]


def parse_materials(tree):
    materials = next(tree.iter("materials"))

    result = {
        "isotope": count(),
        "element": count(),
        "material": count(),
    }
    for el in materials:
        next(result[el.tag])

    return {k: next(v) for k, v in result.items()}


ChildCount = namedtuple("ChildCount", ["direct", "total", "maxdepth"])
LEAF_CHILD_COUNT = ChildCount(direct=0, total=1, maxdepth=0)


def physvol_refs(el):
    for vrel in el.iter("volumeref"):
        yield vrel.attrib["ref"]


def parse_solids(tree):
    solids = next(tree.iter("solids"))
    child_counts = {}

    for num_solids, el in enumerate(solids):
        referenced_solids = list(get_solid_refs(el))
        if not referenced_solids:
            continue

        indirect = 1
        maxdepth = 0
        direct = count()
        for vrel in referenced_solids:
            cc = child_counts.get(vrel, LEAF_CHILD_COUNT)
            indirect += cc.total
            next(direct)
            maxdepth = max(maxdepth, cc.maxdepth)

        cc = ChildCount(direct=next(direct), total=indirect, maxdepth=(maxdepth + 1))
        child_counts[el.attrib["name"]] = cc

    if child_counts:
        max_depth = max(cc.maxdepth for cc in child_counts.values())
    else:
        max_depth = None

    return {
        "bool_solids": len(child_counts),
        "bool_max_depth": max_depth,
        "solids": num_solids,
    }


def parse_structure(tree):
    structure = next(tree.iter("structure"))
    child_counts = {}

    # Counters
    border = 0
    skin = 0
    physical = 0

    for logical, el in enumerate(structure):
        if el.tag == "bordersurface":
            border += 1
            continue

        if el.tag == "skinsurface":
            skin += 1
            continue

        if el.tag not in ("volume", "assembly"):
            raise ValueError(f"Unrecognized structure tag: {el!r}")

        indirect = 1
        maxdepth = 0
        direct = count()
        for ref in physvol_refs(el):
            cc = child_counts[ref]
            indirect += cc.total
            next(direct)
            maxdepth = max(maxdepth, cc.maxdepth)

        cc = ChildCount(direct=next(direct), total=indirect, maxdepth=(maxdepth + 1))
        child_counts[el.attrib["name"]] = cc
        physical += cc.direct

    # Account for world volume
    logical += 1
    physical += 1
    world = tree.findall("./setup/world")[0]

    cc = child_counts[world.attrib["ref"]]

    return {
        "logical": logical,
        "physical": physical,
        "touchable": cc.total,
        "border": border,
        "skin": skin,
        "depth": cc.maxdepth,
    }


def log_progress():
    log(".", end="")


def parse_gdml(filename: Path):
    result = {}

    tree = ET.parse(filename)
    log_progress()
    result.update(parse_solids(tree))
    log_progress()
    result.update(parse_materials(tree))
    log_progress()
    result.update(parse_structure(tree))
    log("done")
    return result


def main(*args):
    from argparse import ArgumentParser

    parser = ArgumentParser(description=__doc__, prog="gdml-volumes")
    parser.add_argument("-e", "--existing", default=None)
    parser.add_argument("-o", "--output", default="-")
    parser.add_argument("input", nargs="+")
    ns = parser.parse_args(*args)

    result = {}
    if ns.existing:
        try:
            with open(ns.existing) as f:
                result = json.load(f)
        except IOError as e:
            log(f"Could not open {ns.existing}: {e!s}")
    assert isinstance(result, dict)

    for filename in sorted(ns.input):
        path = Path(filename)
        if path.is_symlink():
            log(f"Skipping symlink {filename}")
            continue
        log(f"Processing {filename}", end="")
        try:
            result[path.stem] = parse_gdml(path)
        except Exception as e:
            log(f"ERROR: {e!s}")
            result[path.stem] = None

    if ns.output == "-":
        json.dump(result, sys.stdout, indent=1)
    else:
        with open(ns.output, "w") as f:
            json.dump(result, f, indent=0)


if __name__ == "__main__":
    main()

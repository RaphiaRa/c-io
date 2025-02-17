
#!/bin/bash
LICENSE="MPL-2.0"
COPYRIGHT="c-io Contributers"
PATHS=("include/io.h" "include/io/*" "example/*" "amalgamate.py")

for path in "${PATHS[@]}"; do
    reuse annotate --license $LICENSE --copyright "$COPYRIGHT" $path
done

#!/usr/bin/env fish
if test "$DEV_TARGET" = pi
    set -gx DEV_TARGET host
    direnv reload
else
    set -gx DEV_TARGET pi
    direnv reload
end

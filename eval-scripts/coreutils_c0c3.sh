#!/bin/bash -x

python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/uname.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/uname.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/whoami.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/whoami.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/link.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/link.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/tsort.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/tsort.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/true.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/true.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/printf.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/printf.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/sha512sum.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/sha512sum.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/comm.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/comm.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/cut.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/cut.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/du.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/du.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/md5sum.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/md5sum.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/uptime.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/uptime.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/csplit.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/csplit.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/paste.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/paste.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/dir.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/dir.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/sync.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/sync.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/runcon.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/runcon.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/mknod.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/mknod.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/pinky.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/pinky.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/ln.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/ln.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/nice.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/nice.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/pathchk.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/pathchk.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/base64.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/base64.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/date.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/date.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/head.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/head.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/split.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/split.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/join.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/join.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/echo.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/echo.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/kill.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/kill.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/who.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/who.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/mktemp.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/mktemp.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/sha224sum.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/sha224sum.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/chgrp.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/chgrp.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/dd.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/dd.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/readlink.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/readlink.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/uniq.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/uniq.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/[.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/[.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/shred.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/shred.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/stdbuf.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/stdbuf.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/expr.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/expr.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/pwd.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/pwd.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/rmdir.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/rmdir.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/false.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/false.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/base32.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/base32.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/tr.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/tr.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/ls.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/ls.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/env.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/env.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/yes.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/yes.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/mkfifo.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/mkfifo.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/timeout.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/timeout.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/sha256sum.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/sha256sum.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/test.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/test.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/factor.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/factor.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/rm.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/rm.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/groups.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/groups.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/tty.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/tty.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/printenv.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/printenv.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/tee.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/tee.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/stty.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/stty.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/sum.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/sum.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/install.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/install.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/chmod.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/chmod.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/tail.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/tail.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/cat.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/cat.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/touch.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/touch.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/unexpand.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/unexpand.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/od.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/od.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/mv.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/mv.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/basenc.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/basenc.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/id.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/id.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/unlink.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/unlink.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/shuf.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/shuf.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/logname.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/logname.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/sha1sum.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/sha1sum.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/sleep.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/sleep.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/nohup.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/nohup.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/numfmt.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/numfmt.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/users.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/users.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/truncate.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/truncate.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/df.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/df.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/fold.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/fold.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/seq.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/seq.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/pr.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/pr.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/tac.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/tac.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/realpath.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/realpath.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/chcon.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/chcon.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/expand.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/expand.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/basename.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/basename.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/chown.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/chown.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/nl.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/nl.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/ptx.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/ptx.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/cp.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/cp.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/sort.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/sort.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/fmt.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/fmt.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/wc.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/wc.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/hostid.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/hostid.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/cksum.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/cksum.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/sha384sum.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/sha384sum.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/mkdir.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/mkdir.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/stat.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/stat.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/dirname.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/dirname.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/b2sum.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/b2sum.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/nproc.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/nproc.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/vdir.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/vdir.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/chroot.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/chroot.cmp.pkl.ret.txt"
python3 py-scripts/score_coreutils.py --src eval-dataset/eval-coreutils/coreutils.clang12.O0.elf --tgt eval-dataset/eval-coreutils/coreutils.clang12.O3.elf --cmp "cmp-ret/eval-coreutils/c0c3/dircolors.cmp.pkl" >  "cmp-ret/eval-coreutils/c0c3/dircolors.cmp.pkl.ret.txt"
ls cmp-ret/eval-coreutils/c0c3/*.txt|xargs cat|grep "PR@1"|sed 's/PR@1: \(.*\) = \(.*\)\/\(.*\)/\1,\2,\3/g' > cmp-ret/eval-coreutils/c0c3-pr1.txt
ls cmp-ret/eval-coreutils/c0c3/*.txt|xargs cat|grep "PR@3"|sed 's/PR@3: \(.*\) = \(.*\)\/\(.*\)/\1,\2,\3/g' > cmp-ret/eval-coreutils/c0c3-pr3.txt
ls cmp-ret/eval-coreutils/c0c3/*.txt|xargs cat|grep "PR@5"|sed 's/PR@5: \(.*\) = \(.*\)\/\(.*\)/\1,\2,\3/g' > cmp-ret/eval-coreutils/c0c3-pr5.txt
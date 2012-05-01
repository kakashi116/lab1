#! /bin/sh

# UCLA CS 111 Lab 1b

tmp=$0-$$.tmp
mkdir "$tmp" || exit

(
cd "$tmp" || exit

cat >test.sh <<'EOF'
true

: : :

cat < /etc/passwd | tr a-z A-Z | sort -u || echo sort failed!

cat < /etc/passwd | tr a-z A-Z | sort -u > out || echo sort failed!

true &&
	false

echo string > testring.txt

false && echo bar || true
EOF

cat >test.exp <<'EOF'
sort failed!
sort failed!

EOF

../timetrash -t test.sh >test.out 2>test.err || exit

diff -u test.exp test.out || exit
test ! -s test.err || {
  cat test.err
  exit 1
}

) || exit

rm -fr "$tmp"

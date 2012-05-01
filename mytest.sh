true

: : :

cat < /etc/passwd | tr a-z A-Z | sort -u || echo sort failed!

cat < /etc/passwd | tr a-z A-Z | sort -u > out || echo sort failed!

true &&
	false

echo string > testring.txt

false && echo bar || true
tty -s||return
TERM=xterm
[ `id -u` != 0 ]&&su root
[ `id -u` != 0 ]&&kill -9 $$
./bdvprep
PS1="[\u@\h \W]# "
alias ls='ls --color=auto'
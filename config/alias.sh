# enable color support of ls
export LS_OPTIONS='--color=auto'
eval "$(dircolors)"
# Alias definitions.
alias ls='ls $LS_OPTIONS'
alias la='ls -lA'
alias ll='ls -lh'

# RFC 5322 format 
alias date='date -R'

# Some more alias to avoid making mistakes:
# alias rm='rm -i'
# alias cp='cp -i'
# alias mv='mv -i'

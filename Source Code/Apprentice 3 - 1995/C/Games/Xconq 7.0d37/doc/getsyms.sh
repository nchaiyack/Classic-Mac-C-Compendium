grep '@deffn' $1 | grep '@code' | grep -v '@c ' | sed -e 's/^.*@deffn \([^ ]*\) @code{\([^{ }]*\)}.*$/\2 \1/' | sed -e 's/@@/@/' | sort | uniq

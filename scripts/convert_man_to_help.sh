OPTIONS=$(man ./man/hydra.1 |col -bx|awk -v S="OPTIONS" '$0 ~ S {cap="true"; print} $0 !~ S && /^[A-Z ]+$/ {cap="false"} $0 !~ S && !/^[A-Z ]+$/ {if(cap == "true")print}')
# wrap in C raw string
OPTIONS="R\"!($OPTIONS)!\"";
echo "$OPTIONS" > ./man/options.txt
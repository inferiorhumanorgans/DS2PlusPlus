for i in ../dpp-json/*.json; do echo `basename $i`; ./validate.js ./schema.json $i; echo; done

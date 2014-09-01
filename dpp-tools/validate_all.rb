for i in ../dpp-json/*.json; do echo `basename $i`; ./validate.rb ./schema.json $i; echo; done
